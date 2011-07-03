/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011 Michał Męciński
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "viewmanager.h"

#include "viewer/viewerwindow.h"
#include "shell/shellpidl.h"

class ViewItem
{
public:
    ViewItem( const ShellPidl& pidl ) :
        m_pidl( pidl ),
        m_window( NULL ),
        m_view( NULL )
    {
    }

    ViewItem() :
        m_window( NULL ),
        m_view( NULL )
    {
    }

    ~ViewItem()
    {
    }

public:
    ShellPidl m_pidl;
    ViewerWindow* m_window;
    View* m_view;
};

class ViewManagerPrivate
{
public:
    ViewManagerPrivate();
    ~ViewManagerPrivate();

public:
    QList<ViewItem> m_items;
};

ViewManager::ViewManager() :
    d( new ViewManagerPrivate() )
{
}

ViewManager::~ViewManager()
{
    delete d;
}

ViewManagerPrivate::ViewManagerPrivate()
{
}

ViewManagerPrivate::~ViewManagerPrivate()
{
}

void ViewManager::openView( const ShellPidl& pidl )
{
    foreach ( const ViewItem& item, d->m_items ) {
        if ( item.m_pidl == pidl ) {
            item.m_window->raise();
            item.m_window->activateWindow();
            return;
        }
    }

    QByteArray format;
    View::Type type;
    bool ok = checkType( pidl, View::Auto, type, format );

    ViewItem item( pidl );
    item.m_window = new ViewerWindow();
    item.m_view = View::createView( type, item.m_window, item.m_window );

    d->m_items.append( item );

    connect( item.m_window, SIGNAL( destroyed( QObject* ) ), this, SLOT( windowDestroyed( QObject* ) ) );

    QFileInfo info( pidl.path() );
    item.m_window->setWindowTitle( QString( "%1 - Saladin" ).arg( info.fileName() ) );

    item.m_window->setView( item.m_view );
    item.m_window->showMaximized();

    if ( ok )
        item.m_view->load( pidl.path(), format );
}

void ViewManager::switchViewType( ViewerWindow* window, View::Type type )
{
    for ( int i = 0; i < d->m_items.count(); i++ ) {
        ViewItem& item = d->m_items[ i ];

        if ( item.m_window == window ) {
            QByteArray format;
            bool ok = checkType( item.m_pidl, type, type, format );

            item.m_view = View::createView( type, window, window );
            item.m_window->setView( item.m_view );

            if ( ok )
                item.m_view->load( item.m_pidl.path(), format );
            break;
        }
    }
}

void ViewManager::windowDestroyed( QObject* window )
{
    for ( int i = 0; i < d->m_items.count(); i++ ) {
        if ( d->m_items.at( i ).m_window == window ) {
            d->m_items.removeAt( i );
            break;
        }
    }
}

static bool checkUtf16( const QByteArray& header, QByteArray& format )
{
    // this heuristics works for files containing mostly 0000-06FF characters,
    // which include latin, greek, cyrillic, hebrew and arabic letters

    int le = 0;
    int be = 0;

    for ( int i = 0; i < header.length(); i += 2 ) {
        uchar lo = (uchar)header.at( i );
        uchar hi = (uchar)header.at( i + 1 );

        if ( lo == 0 && hi == 0 || lo == 255 && hi == 255 )
            return false;

        if ( hi == 0 )
            le++;
        else if ( hi > 6 )
            le--;

        if ( lo == 0 )
            be++;
        else if ( lo > 6 )
            be--;
    }

    if ( le > 0 && be < 0 ) {
        format = "UTF-16LE";
        return true;
    }

    if ( be > 0 && le < 0 ) {
        format = "UTF-16BE";
        return true;
    }

    return false;
}

static bool checkUtf8( const QByteArray& header )
{
    QTextCodec* codec = QTextCodec::codecForName( "UTF-8" );

    QTextCodec::ConverterState state;
    codec->toUnicode( header.data(), header.length(), &state );

    if ( state.invalidChars == 0 )
        return true;

    return false;
}

static bool checkBinary( const QByteArray& header )
{
    for ( int i = 0; i < header.length(); i++ ) {
        uchar ch = (uchar)header.at( i );
        if ( ch < 9 || ch > 13 && ch < 32 )
            return true;
    }

    return false;
}

bool ViewManager::checkType( const ShellPidl& pidl, View::Type inType, View::Type& outType, QByteArray& format )
{
    if ( inType == View::Auto )
        outType = View::Binary;
    else
        outType = inType;

    QFile file( pidl.path() );

    if ( !file.open( QIODevice::ReadOnly ) )
        return false;

    QByteArray header = file.peek( 512 );
    if ( header.isEmpty() )
        return false;

    if ( inType == View::Binary )
        return true;

    if ( inType == View::Auto || inType == View::Image ) {
        outType = View::Image;

        QImageReader reader( &file );

        if ( inType == View::Auto ) {
            reader.setFormat( QFileInfo( file ).suffix().toLower().toLatin1() );
            reader.setAutoDetectImageFormat( false );
        }

        if ( reader.canRead() ) {
            format = reader.format();
            return true;
        }
    }

    if ( inType == View::Auto || inType == View::Text ) {
        outType = View::Text;

        QTextCodec* codec = QTextCodec::codecForUtfText( header, NULL );
        if ( codec != NULL ) {
            format = codec->name();
            return true;
        }

        if ( file.size() % 2 == 0 ) {
            if ( checkUtf16( header, format ) )
                return true;
        }

        if ( inType == View::Auto ) {
            if ( checkBinary( header ) ) {
                outType = View::Binary;
                return true;
            }
        }

        if ( checkUtf8( header ) ) {
            format = "UTF-8";
            return true;
        }

        format = QTextCodec::codecForLocale()->name();
        return true;
    }

    return false;
}
