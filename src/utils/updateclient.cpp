/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2010 Michał Męciński
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

#include "updateclient.h"

UpdateClient::UpdateClient( const QString& application, const QString& version, QObject* parent ) : QObject( parent ),
    m_currentReply( NULL ),
    m_timer( NULL ),
    m_application( application ),
    m_version( version ),
    m_state( ErrorState )
{
    m_manager = new QNetworkAccessManager( this );

    connect( m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( finished( QNetworkReply* ) ) );
}

UpdateClient::~UpdateClient()
{
}

void UpdateClient::setAutoUpdate( bool enabled )
{
    if ( autoUpdate() == enabled )
        return;

    if ( enabled ) {
        m_timer = new QTimer( this );
        m_timer->start( 4 * 3600 * 1000 );
        
        connect( m_timer, SIGNAL( timeout() ), this, SLOT( checkUpdate() ) );

        checkUpdate();
    } else {
        delete m_timer;
        m_timer = NULL;
    }
}

void UpdateClient::checkUpdate()
{
    if ( m_state == CheckingState )
        return;

    m_state = CheckingState;

    m_updateVersion.clear();
    m_notesUrl.clear();
    m_downloadUrl.clear();

    QList<QPair<QString, QString> > query;
    query.append( QPair<QString, QString>( "app", m_application ) );
    query.append( QPair<QString, QString>( "ver", m_version ) );

    QUrl url( "http://update.mimec.org/service.php" );
    url.setQueryItems( query );

    m_currentReply = m_manager->get( QNetworkRequest( url ) );

    emit stateChanged();
}

void UpdateClient::finished( QNetworkReply* reply )
{
    if ( reply != m_currentReply )
        return;

    m_currentReply = NULL;

    int statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toByteArray();

    if ( reply->error() == QNetworkReply::NoError && statusCode == 200 && contentType.startsWith( "text/xml" ) ) {
        QDomDocument document;

        if ( document.setContent( reply, false ) ) {
            QDomElement version = document.documentElement().firstChildElement( "version" );

            if ( !version.isNull() ) {
                m_state = UpdateAvailableState;

                m_updateVersion = version.attribute( "id" );
                m_notesUrl = QUrl( version.firstChildElement( "notesUrl" ).text() );
                m_downloadUrl = QUrl( version.firstChildElement( "downloadUrl" ).text() );
            } else {
                m_state = CurrentVersionState;
            }
        } else {
            m_state = ErrorState;
        }
    } else {
        m_state = ErrorState;
    }

    reply->deleteLater();

    emit stateChanged();
}
