/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2017 Michał Męciński
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

#ifndef UPDATECLIENT_H
#define UPDATECLIENT_H

#include <QObject>

class UpdateClient : public QObject
{
    Q_OBJECT
public:
    enum State {
        ErrorState,
        CheckingState,
        CurrentVersionState,
        UpdateAvailableState
    };

public:
    UpdateClient( const QString& application, const QString& version, QNetworkAccessManager* manager );
    ~UpdateClient();

public:
    void setAutoUpdate( bool enabled );
    bool autoUpdate() const { return m_timer != NULL; }

    State state() const { return m_state; }

    const QString& updateVersion() const { return m_updateVersion; }

    const QUrl& notesUrl() const { return m_notesUrl; }
    const QUrl& downloadUrl() const { return m_downloadUrl; }

public slots:
    void checkUpdate();

signals:
    void stateChanged();

private slots:
    void finished( QNetworkReply* reply );

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_currentReply;

    QTimer* m_timer;

    QString m_application;
    QString m_version;

    State m_state;

    QString m_updateVersion;
    QUrl m_notesUrl;
    QUrl m_downloadUrl;
};

#endif
