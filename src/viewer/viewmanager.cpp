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
    QString name;
    bool ok = checkType( pidl, View::Auto, type, format, name );

    ViewItem item( pidl );
    item.m_window = new ViewerWindow();
    item.m_view = View::createView( type, item.m_window, item.m_window );

    d->m_items.append( item );

    connect( item.m_window, SIGNAL( destroyed( QObject* ) ), this, SLOT( windowDestroyed( QObject* ) ) );

    if ( name.isEmpty() )
        name = tr( "Unknown file" );

    item.m_window->setWindowTitle( QString( "%1 - Saladin" ).arg( name ) );

    item.m_window->setView( item.m_view );
    item.m_window->show();

    if ( ok ) {
        item.m_view->setPidl( pidl );
        item.m_view->setFormat( format );
        item.m_view->load();
    }
}

void ViewManager::switchViewType( ViewerWindow* window, View::Type type )
{
    for ( int i = 0; i < d->m_items.count(); i++ ) {
        ViewItem& item = d->m_items[ i ];

        if ( item.m_window == window ) {
            QByteArray format;
            QString name;
            bool ok = checkType( item.m_pidl, type, type, format, name );

            item.m_view = View::createView( type, window, window );
            item.m_window->setView( item.m_view );

            if ( ok ) {
                item.m_view->setPidl( item.m_pidl );
                item.m_view->setFormat( format );
                item.m_view->load();
            }

            item.m_view->mainWidget()->setFocus();
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
