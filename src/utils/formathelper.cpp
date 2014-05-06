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

#include "formathelper.h"
#include "shell/streamdevice.h"

bool FormatHelper::checkImage( StreamDevice& file, bool force, QByteArray& format )
{
    QImageReader reader( &file );

    if ( !force ) {
        QByteArray format;
        QString name = file.name();
        int pos = name.lastIndexOf( QLatin1Char( '.' ) );
        if ( pos > 0 )
            format = name.mid( pos + 1 ).toLower().toLatin1();
        reader.setFormat( format );
        reader.setAutoDetectImageFormat( false );
    }

    if ( reader.canRead() ) {
        format = reader.format();
        return true;
    }

    return false;
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

bool FormatHelper::checkText( QIODevice& file, bool force, QByteArray& format )
{
    QByteArray header = file.peek( 512 );
    if ( header.isEmpty() )
        return false;

    QTextCodec* codec = QTextCodec::codecForUtfText( header, NULL );
    if ( codec != NULL ) {
        format = codec->name();
        return true;
    }

    if ( file.size() % 2 == 0 ) {
        if ( checkUtf16( header, format ) )
            return true;
    }

    if ( !force && checkBinary( header ) )
        return false;

    if ( checkUtf8( header ) ) {
        format = "UTF-8";
        return true;
    }

    format = QTextCodec::codecForLocale()->name();
    return true;
}
