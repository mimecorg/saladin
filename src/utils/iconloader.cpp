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

#include "iconloader.h"

#include <QPixmapCache>
#include <QPainter>
#include <QFile>

QPixmap IconLoader::pixmap( const QString& name, int size )
{
    QString key = QString( "pixmap:%1:%2" ).arg( name ).arg( size );

    QPixmap pixmap;
    if ( QPixmapCache::find( key, pixmap ) )
        return pixmap;

    QString path = QString( ":/icons/%1-%2.png" ).arg( name ).arg( size );

    pixmap = QPixmap( path );

    if ( !pixmap.isNull() )
        QPixmapCache::insert( key, pixmap );

    return pixmap;
}

QPixmap IconLoader::overlayedPixmap( const QString& name, const QString& overlay, int size )
{
    QString key = QString( "overlayed:%1:%2:%3" ).arg( name ).arg( overlay ).arg( size );

    QPixmap resultPixmap;
    if ( QPixmapCache::find( key, resultPixmap ) )
        return resultPixmap;

    QPixmap basePixmap = pixmap( name, size );
    QPixmap overlayPixmap = pixmap( overlay, size );

    resultPixmap = QPixmap( size, size );
    resultPixmap.fill( QColor( 0, 0, 0, 0 ) );

    QPainter painter( &resultPixmap );
    painter.drawPixmap( 0, 0, basePixmap );
    painter.drawPixmap( 0, 0, overlayPixmap );

    QPixmapCache::insert( key, resultPixmap );

    return resultPixmap;
}

QPixmap IconLoader::overlayedPixmap( const QString& name, const QString& overlay1, const QString& overlay2, int size )
{
    QString key = QString( "overlayed2:%1:%2:%3:%4" ).arg( name ).arg( overlay1 ).arg( overlay2 ).arg( size );

    QPixmap resultPixmap;
    if ( QPixmapCache::find( key, resultPixmap ) )
        return resultPixmap;

    QPixmap basePixmap = pixmap( name, size );
    QPixmap overlay1Pixmap = pixmap( overlay1, size );
    QPixmap overlay2Pixmap = pixmap( overlay2, size );

    resultPixmap = QPixmap( size, size );
    resultPixmap.fill( QColor( 0, 0, 0, 0 ) );

    QPainter painter( &resultPixmap );
    painter.drawPixmap( 0, 0, basePixmap );
    painter.drawPixmap( 0, 0, overlay1Pixmap );
    painter.drawPixmap( 0, 0, overlay2Pixmap );

    QPixmapCache::insert( key, resultPixmap );

    return resultPixmap;
}

QIcon IconLoader::icon( const QString& name )
{
    QIcon icon;

    const int sizes[ 4 ] = { 16, 22, 32, 48 };

    for ( int i = 0; i < 4; i++ ) {
        QString path = QString( ":/icons/%1-%2.png" ).arg( name ).arg( sizes[ i ] );

        if ( QFile::exists( path ) )
            icon.addFile( path, QSize( sizes[ i ], sizes[ i ] ) );
    }

    return icon;
}

QIcon IconLoader::overlayedIcon( const QString& name, const QString& overlay )
{
    QIcon icon;

    const int sizes[ 4 ] = { 16, 22, 32, 48 };

    for ( int i = 0; i < 4; i++ ) {
        QString imagePath = QString( ":/icons/%1-%2.png" ).arg( name ).arg( sizes[ i ] );
        QString overlayPath = QString( ":/icons/%1-%2.png" ).arg( overlay ).arg( sizes[ i ] );

        if ( QFile::exists( imagePath ) && QFile::exists( overlayPath ) )
            icon.addPixmap( overlayedPixmap( name, overlay, sizes[ i ] ) );
    }

    return icon;
}
