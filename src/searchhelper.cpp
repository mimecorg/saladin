/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2012 Michał Męciński
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

#include "searchhelper.h"

#include "utils/formathelper.h"

SearchHelper::SearchHelper( const QString& pattern, Qt::CaseSensitivity cs ) :
    m_matcher( pattern, cs ),
    m_index( -1 ),
    m_aborted( false )
{
}

SearchHelper::~SearchHelper()
{
}

void SearchHelper::search( const QStringList& files )
{
    m_mutex.lock();

    m_files = files;
    m_results.clear();
    m_index = -1;

    m_mutex.unlock();

    m_condition.wakeAll();
}

void SearchHelper::run()
{
    m_mutex.lock();

    while ( !m_aborted ) {
        if ( m_files.isEmpty() )
            m_condition.wait( &m_mutex );

        while ( !m_files.isEmpty() && !m_aborted ) {
            QString path = m_files.takeFirst();
            m_index++;

            m_mutex.unlock();
            
            bool found = false;

            QFile file( path );

            if ( file.open( QIODevice::ReadOnly ) ) {
                QByteArray format;

                if ( FormatHelper::checkText( file, false, format ) ) {
                    QTextStream stream( &file );
                    stream.setAutoDetectUnicode( false );
                    stream.setCodec( format );

                    QString remainder;

                    while ( !stream.atEnd() && !m_aborted && !found ) {
                        QString block = stream.read( 16 * 1024 );
                    
                        block = remainder + block;

                        if ( m_matcher.indexIn( block ) >= 0 )
                            found = true;

                        remainder = block.right( m_matcher.pattern().length() - 1 );
                    }
                }
            }

            m_mutex.lock();
            
            if ( found && m_index >= 0 )
                m_results.append( m_index );
        }

        if ( !m_aborted )
            emit completed();
    }

    m_mutex.unlock();
}

QList<int> SearchHelper::results()
{
    m_mutex.lock();

    QList<int> results = m_results;
    m_results.clear();

    m_mutex.unlock();

    return results;
}

void SearchHelper::abort()
{
    m_aborted = true;

    m_condition.wakeAll();

    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );

    if ( isFinished() )
        deleteLater();
}
