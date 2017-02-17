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

#include "imageloader.h"
#include "shell/streamdevice.h"

ImageLoader::ImageLoader( const ShellPidl& pidl ) :
    m_pidl( pidl ),
    m_aborted( false )
{
}

ImageLoader::~ImageLoader()
{
}

void ImageLoader::run()
{
    StreamDevice file( m_pidl );

    if ( file.open( QIODevice::ReadOnly ) ) {
        QImageReader reader( &file );

        reader.setAutoTransform( true );

        m_format = reader.format();

        if ( reader.read( &m_image ) && !m_aborted )
            emit imageAvailable();
    }
}

void ImageLoader::abort()
{
    m_aborted = true;

    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );

    if ( isFinished() )
        deleteLater();
}
