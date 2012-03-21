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

#include "textloader.h"

TextLoader::TextLoader( const QString& path, const QByteArray& format ) :
    m_path( path ),
    m_format( format ),
    m_aborted( false ),
    m_signal( false ),
    m_atEnd( false ),
    m_estimatedLength( 0 )
{
}

TextLoader::~TextLoader()
{
}

void TextLoader::run()
{
    QFile file( m_path );

    if ( file.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &file );
        stream.setAutoDetectUnicode( false );
        stream.setCodec( m_format );

        qint64 length = 0;

        while ( !m_aborted && !m_atEnd ) {
            QString block = stream.read( 16 * 1024 );

            length += block.size();

            m_mutex.lock();

            bool signal = m_signal;
            m_signal = false;

            if ( !block.isEmpty() )
                m_queue.enqueue( block );

            m_estimatedLength = file.size() * length / file.pos();

            if ( stream.atEnd() )
                m_atEnd = true;

            m_mutex.unlock();

            if ( signal && !m_aborted )
                emit nextBlockAvailable();
        }
    }
}

QString TextLoader::nextBlock()
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

void TextLoader::abort()
{
    m_aborted = true;

    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );

    if ( isFinished() )
        deleteLater();
}
