/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2013 Michał Męciński
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

#include "binaryloader.h"

BinaryLoader::BinaryLoader( const QString& path, bool hexMode ) :
    m_path( path ),
    m_hexMode( hexMode ),
    m_aborted( false ),
    m_signal( false ),
    m_atEnd( false ),
    m_estimatedLength( 0 )
{
}

BinaryLoader::~BinaryLoader()
{
}

void BinaryLoader::run()
{
    QFile file( m_path );

    if ( file.open( QIODevice::ReadOnly ) ) {
        QDataStream stream( &file );

        char input[ 256 ];
        for ( int i = 0; i < 256; i++ )
            input[ i ] = i;

        QTextCodec* codec = QTextCodec::codecForLocale();
        QString output = codec->toUnicode( input, 256 );
        QChar* map = output.data();

        for ( int i = 0; i < 256; i++ ) {
            if ( !map[ i ].isPrint() )
                map[ i ] = QLatin1Char( '.' );
            else if ( map[ i ].unicode() == 173 )
                map[ i ] = QLatin1Char( '-' );
        }

        int lineSize = m_hexMode ? 16 : 80;
        int offset = m_hexMode ? 61 : 0;

        char buffer[ 16000 ];

        QChar hex[ 16 ];
        QChar line[ 81 ];

        if ( m_hexMode ) {
            for ( int i = 0; i < 10; i++ )
                hex[ i ] = QLatin1Char( i + '0' );
            for ( int i = 0; i < 6; i++ )
                hex[ i + 10 ] = QLatin1Char( i + 'A' );

            for ( int i = 0; i < 81; i++ )
                line[ i ] = QLatin1Char( ' ' );
            line[ 9 ] = QLatin1Char( '|' );
            line[ 59 ] = QLatin1Char( '|' );
        }

        qint64 pos = 0;

        while ( !m_aborted && !m_atEnd ) {
            int len = stream.readRawData( buffer, 16000 );

            QString block;
            block.reserve( ( 16000 / lineSize ) * ( lineSize + offset + 1 ) );

            for ( int start = 0; start < len; start += lineSize ) {
                char* lbuffer = buffer + start;
                int llen = qMin( lineSize, len - start );

                if ( m_hexMode ) {
                    uint p = (uint)pos;
                    for ( int i = 7; i >= 0; i-- ) {
                        line[ i ] = hex[ p & 0xf ];
                        p >>= 4;
                    }

                    for ( int i = 0; i < llen; i++ ) {
                        uchar ch = (uchar)lbuffer[ i ];
                        line[ 3 * i + 11 ] = hex[ ( ch >> 4 ) & 0xf ];
                        line[ 3 * i + 12 ] = hex[ ch & 0xf ];
                    }

                    for ( int i = llen; i < 16; i++ ) {
                        line[ 3 * i + 11 ] = QLatin1Char( ' ' );
                        line[ 3 * i + 12 ] = QLatin1Char( ' ' );
                    }
                }

                for ( int i = 0; i < llen; i++ )
                    line[ i + offset ] = map[ (uchar)lbuffer[ i ] ];
                line[ llen + offset ] = QLatin1Char( '\n' );

                block.append( QString::fromRawData( line, llen + offset + 1 ) );

                pos += llen;
            }

            m_mutex.lock();

            bool signal = m_signal;
            m_signal = false;

            if ( !block.isEmpty() )
                m_queue.enqueue( block );

            m_size = file.size();
            m_estimatedLength = ( m_size / lineSize + 1 ) * ( lineSize + offset + 1 );

            if ( stream.atEnd() )
                m_atEnd = true;

            m_mutex.unlock();

            if ( signal && !m_aborted )
                emit nextBlockAvailable();
        }
    }
}

QString BinaryLoader::nextBlock()
{
    QString block;

    m_mutex.lock();

    if ( !m_queue.isEmpty() )
        block = m_queue.dequeue();
    else
        m_signal = true;

    m_mutex.unlock();

    return block;
}

void BinaryLoader::abort()
{
    m_aborted = true;

    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );

    if ( isFinished() )
        deleteLater();
}
