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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "bookmark.h"

#include <QApplication>

class MainWindow;
class LocalSettings;
class UpdateClient;
class AboutBox;
class AboutBoxSection;
class GuideDialog;

class Application : public QApplication
{
    Q_OBJECT
public:
    Application( int& argc, char** argv );
    ~Application();

public:
    QString version() const;

    QString translationsPath() const { return m_translationsPath; }

    QString locateDataFile( const QString& name );
    QString locateCacheFile( const QString& name );
    QString locateTempFile( const QString& name );

    LocalSettings* applicationSettings() const { return m_settings; }

    const QList<Bookmark>& bookmarks() const { return m_bookmarks; }
    void setBookmarks( const QList<Bookmark>& bookmarks );
    void addBookmark( const Bookmark& bookmark );

public slots:
    void about();

    void showQuickGuide();

    void showUpdateState();

    void openDonations();
    void openReleaseNotes();
    void openDownloads();

    void settingsChanged();

private:
    bool loadTranslation( const QString& name, bool tryQtDir );

    void initializeDefaultPaths();

    void initializeSettings();

    bool checkAccess( const QString& path );

    void loadBookmarks();
    void saveBookmarks();

    QString technicalInformation();

private:
    QString m_translationsPath;

    QString m_dataPath;
    QString m_cachePath;
    QString m_tempPath;

    LocalSettings* m_settings;

    QList<Bookmark> m_bookmarks;

    UpdateClient* m_updateClient;

    QPointer<AboutBox> m_aboutBox;

    QPointer<AboutBoxSection> m_updateSection;
    QPointer<QPushButton> m_updateButton;

    QString m_shownVersion;

    QPointer<GuideDialog> m_guideDialog;
};

extern Application* application;

#endif
