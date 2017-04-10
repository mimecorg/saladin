/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2014 Michał Męciński
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
#include "shell/streamdevice.h"
#include "utils/formathelper.h"

class ViewItem
{
public:
    ViewItem( const QList<ShellPidl>& pidls ) :
        m_pidls( pidls ),
        m_index( 0 ),
        m_window( NULL ),
        m_view( NULL )
    {
    }

    ViewItem() :
        m_index( -1 ),
        m_window( NULL ),
        m_view( NULL )
    {
    }

    ~ViewItem()
    {
    }

public:
    QList<ShellPidl> m_pidls;
    int m_index;
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
    openView( QList<ShellPidl>() << pidl );
}

void ViewManager::openView( const QList<ShellPidl>& pidls )
{
    if ( pidls.count() == 1 ) {
        foreach ( const ViewItem& item, d->m_items ) {
            if ( item.m_pidls.at( item.m_index ) == pidls.first() ) {
                item.m_window->raise();
                item.m_window->activateWindow();
                return;
            }
        }
    }

    QByteArray format;
    View::Type type;
    QString name;
    bool ok = checkType( pidls.first(), View::Auto, type, format, name );

    ViewItem item( pidls );
    item.m_window = new ViewerWindow();
    item.m_view = View::createView( type, item.m_window, item.m_window );

    d->m_items.append( item );

    connect( item.m_window, SIGNAL( destroyed( QObject* ) ), this, SLOT( windowDestroyed( QObject* ) ) );

    updateTitle( item, name );

    item.m_window->enableNavigation( true, pidls.count() == 1 );

    item.m_window->setView( item.m_view );
    item.m_window->show();

    if ( ok ) {
        item.m_view->setPidl( pidls.first() );
        item.m_view->setFormat( format );
        item.m_view->load();
    }
}

void ViewManager::loadPrevious( ViewerWindow* window )
{
    for ( int i = 0; i < d->m_items.count(); i++ ) {
        ViewItem& item = d->m_items[ i ];

        if ( item.m_window == window ) {
            if ( item.m_index > 0 ) {
                item.m_index--;
                loadView( item, View::Auto );
                break;
            }
        }
    }
}

void ViewManager::loadNext( ViewerWindow* window )
{
    for ( int i = 0; i < d->m_items.count(); i++ ) {
        ViewItem& item = d->m_items[ i ];

        if ( item.m_window == window ) {
            if ( item.m_index < item.m_pidls.count() - 1 ) {
                item.m_index++;
                loadView( item, View::Auto );
                break;
            }
        }
    }
}

void ViewManager::switchViewType( ViewerWindow* window, View::Type type )
{
    for ( int i = 0; i < d->m_items.count(); i++ ) {
        ViewItem& item = d->m_items[ i ];

        if ( item.m_window == window ) {
            loadView( item, type );
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

void ViewManager::loadView( ViewItem& item, View::Type type )
{
    QByteArray format;
    QString name;
    bool ok = checkType( item.m_pidls.at( item.m_index ), type, type, format, name );

    item.m_view->storeSettings();

    item.m_view = View::createView( type, item.m_window, item.m_window );
    item.m_window->setView( item.m_view );

    updateTitle( item, name );

    item.m_window->enableNavigation( item.m_index == 0, item.m_index == item.m_pidls.count() - 1 );

    if ( ok ) {
        item.m_view->setPidl( item.m_pidls.at( item.m_index ) );
        item.m_view->setFormat( format );
        item.m_view->load();
    }

    item.m_view->mainWidget()->setFocus();
}

void ViewManager::updateTitle( ViewItem& item, const QString& name )
{
    QString title = name;

    if ( title.isEmpty() )
        title = tr( "Unknown file" );

    if ( item.m_pidls.count() > 1 )
        title += QString( " - " ) + tr( "%1 of %2" ).arg( item.m_index + 1 ).arg( item.m_pidls.count() );

    item.m_window->setWindowTitle( QString( "%1 - Saladin" ).arg( title ) );
}

bool ViewManager::checkType( const ShellPidl& pidl, View::Type inType, View::Type& outType, QByteArray& format, QString& name )
{
    if ( inType == View::Auto )
        outType = View::Binary;
    else
        outType = inType;

    StreamDevice file( pidl );

    if ( !file.open( QIODevice::ReadOnly ) )
        return false;

    name = file.name();

    if ( inType == View::Binary )
        return true;

    if ( inType == View::Auto || inType == View::Image ) {
        outType = View::Image;

        if ( FormatHelper::checkImage( file, inType == View::Image, format ) )
            return true;
    }

    if ( inType == View::Auto || inType == View::Text ) {
        outType = View::Text;

        if ( FormatHelper::checkText( file, inType == View::Text, format ) )
            return true;

        if ( inType == View::Auto ) {
            outType = View::Binary;
            return true;
        }
    }

    return false;
}
