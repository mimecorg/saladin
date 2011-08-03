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

#include "view.h"

#include "viewer/textview.h"
#include "viewer/binaryview.h"
#include "viewer/imageview.h"

View::View( QObject* parent ) : QObject( parent ),
    m_mainWidget( NULL )
{
}

View::~View()
{
    delete m_mainWidget;
}

void View::setMainWidget( QWidget* widget )
{
    m_mainWidget = widget;
}

void View::setPath( const QString& path )
{
    m_path = path;
}

void View::setFormat( const QByteArray& format )
{
    m_format = format;
}

void View::setStatus( const QString& status )
{
    if ( m_status != status ) {
        m_status = status;
        emit statusChanged( status );
    }
}

View* View::createView( Type type, QObject* parent, QWidget* parentWidget )
{
    switch ( type ) {
        case Text:
            return new TextView( parent, parentWidget );
        case Binary:
            return new BinaryView( parent, parentWidget );
        case Image:
            return new ImageView( parent, parentWidget );
        default:
            return NULL;
    }
}
