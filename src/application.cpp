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

#include "application.h"
#include "mainwindow.h"
#include "aboutbox.h"
#include "guidedialog.h"

#include "utils/localsettings.h"
#include "utils/dataserializer.h"
#include "utils/updateclient.h"
#include "utils/iconloader.h"
#include "shell/shellfolder.h"
#include "shell/shellpidl.h"
#include "xmlui/windowsstyle.h"

Application* application = NULL;

Application::Application( int& argc, char** argv ) : QApplication( argc, argv )
{
    Q_INIT_RESOURCE( icons );

    initializeDefaultPaths();

    application = this;

    ShellPidl::registerMetaType();

    m_settings = new LocalSettings( locateDataFile( "settings.dat" ), this );
    initializeSettings();

    loadBookmarks();

    QString language = m_settings->value( "Language" ).toString();
    if ( language.isEmpty() )
        language = QLocale::system().name();
    QLocale::setDefault( QLocale( language ) );

    loadTranslation( "qt", true );
    loadTranslation( "saladin", false );

    setStyle( new XmlUi::WindowsStyle() );

    setWindowIcon( IconLoader::icon( "saladin" ) );

    initializePalette();

    mainWindow = new MainWindow();

    QNetworkProxyFactory::setUseSystemConfiguration( true );

    QNetworkAccessManager* manager = new QNetworkAccessManager( this );

    m_updateClient = new UpdateClient( "saladin", version(), manager );

    connect( m_updateClient, SIGNAL( stateChanged() ), this, SLOT( showUpdateState() ) );

    settingsChanged();

    connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    if ( m_settings->value( "LastVersion" ).toString() != version() ) {
        m_settings->setValue( "LastVersion", version() );
        QTimer::singleShot( 100, this, SLOT( about() ) );
    }
}

Application::~Application()
{
    delete mainWindow;
    mainWindow = NULL;

    delete m_updateSection;

    delete m_settings;
    m_settings = NULL;
}

void Application::about()
{
    if ( !m_aboutBox ) {
        QString message;
        message += "<h3>" + tr( "Saladin %1" ).arg( version() ) + "</h3>";
        message += "<p>" + trUtf8( "Copyright &copy; 2011-2017 Michał Męciński" ) + " (<a href=\"https://twitter.com/MichalMecinski\">@MichalMecinski</a>)<br>";
        message += trUtf8( "Icons copyright &copy; 2017 Łukasz Grabowski" ) + " (<a href=\"https://twitter.com/LGrabowskiPL\">@LGrabowskiPL</a>)</p>";

        QString notesMessage;
        notesMessage += "<h4>" + tr( "Release Notes" ) + "</h4>";
        notesMessage += "<p>" + tr( "See what's new in this version of Saladin:" );
        notesMessage += QString( " <a href=\"http://saladin.mimec.org/release/%1\">http://saladin.mimec.org/release/%1</a></p>" ).arg( version() );

        QString helpMessage;
        helpMessage += "<h4>" + tr( "Help" ) + "</h4>";
        helpMessage += "<p>" + tr( "Open the Saladin Quick Guide for help." ) + "</p>";

        QString updateMessage;
        updateMessage += "<h4>" + tr( "Latest Version" ) + "</h4>";
        updateMessage += "<p>" + tr( "Automatic checking for latest version is disabled. You can enable it in program settings." ) + "</p>";

        QString licenseMessage;
        licenseMessage += "<h4>" + tr( "License" ) + "</h4>";
        licenseMessage += "<p>" + tr( "This program is free software: you can redistribute it and/or modify"
            " it under the terms of the GNU General Public License as published by"
            " the Free Software Foundation, either version 3 of the License, or"
            " (at your option) any later version." ) + "</p>";

        m_aboutBox = new AboutBox( tr( "About Saladin" ), message, mainWindow );

        m_notesSection = m_aboutBox->addSection( IconLoader::pixmap( "web" ), notesMessage );

        m_helpSection = m_aboutBox->addSection( IconLoader::pixmap( "help" ), helpMessage );

        QPushButton* helpButton = m_helpSection->addButton( tr( "&Quick Guide" ) );
        connect( helpButton, SIGNAL( clicked() ), this, SLOT( showQuickGuide() ) );

        delete m_updateSection;

        m_updateSection = m_aboutBox->addSection( IconLoader::pixmap( "info" ), updateMessage );

        if ( m_updateClient->autoUpdate() ) {
            showUpdateState();
        } else {
            m_updateButton = m_updateSection->addButton( tr( "&Check Now" ) );
            connect( m_updateButton, SIGNAL( clicked() ), m_updateClient, SLOT( checkUpdate() ) );
        }

        m_licenseSection = m_aboutBox->addSection( IconLoader::pixmap( "license" ), licenseMessage );
    }

    m_aboutBox->show();
    m_aboutBox->activateWindow();
}

void Application::showQuickGuide()
{
    if ( m_aboutBox )
        m_aboutBox->close();

    if ( !m_guideDialog ) {
        m_guideDialog = new GuideDialog( mainWindow );

        m_guideDialog->resize( mainWindow->width() / 2, mainWindow->height() - 100 );
        m_guideDialog->move( mainWindow->pos().x() + mainWindow->width() / 2 - 50, mainWindow->pos().y() + 50 );
    }

    m_guideDialog->show();
    m_guideDialog->activateWindow();
}

void Application::showUpdateState()
{
    if ( !m_updateSection ) {
        if ( m_updateClient->state() != UpdateClient::UpdateAvailableState || m_updateClient->updateVersion() == m_shownVersion )
            return;

        m_updateSection = new AboutBoxToolSection();
    } else {
        m_updateSection->clearButtons();
    }

    QString header = "<h4>" + tr( "Latest Version" ) + "</h4>";

    switch ( m_updateClient->state() ) {
        case UpdateClient::CheckingState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "info" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "Checking for latest version..." ) + "</p>" );
            break;
        }

        case UpdateClient::ErrorState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "warning" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "Checking for latest version failed." ) + "</p>" );

            m_updateButton = m_updateSection->addButton( tr( "&Retry" ) );
            connect( m_updateButton, SIGNAL( clicked() ), m_updateClient, SLOT( checkUpdate() ) );
            break;
        }

        case UpdateClient::CurrentVersionState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "info" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "Your version of Saladin is up to date." ) + "</p>" );
            break;
        }

        case UpdateClient::UpdateAvailableState: {
            m_updateSection->setPixmap( IconLoader::pixmap( "warning" ) );
            m_updateSection->setMessage( header + "<p>" + tr( "The latest version of Saladin is %1." ).arg( m_updateClient->updateVersion() ) + "</p>" );

            QPushButton* notesButton = m_updateSection->addButton( tr( "&Release Notes" ) );
            connect( notesButton, SIGNAL( clicked() ), this, SLOT( openReleaseNotes() ) );

            QPushButton* downloadButton = m_updateSection->addButton( tr( "Do&wnload" ) );
            connect( downloadButton, SIGNAL( clicked() ), this, SLOT( openDownloads() ) );

            m_shownVersion = m_updateClient->updateVersion();
            break;
        }
    }
}

void Application::openWebsite()
{
    QDesktopServices::openUrl( QString( "http://saladin.mimec.org/release/%1" ).arg( version() ) );
}

void Application::openDonations()
{
    QDesktopServices::openUrl( QUrl( "http://saladin.mimec.org/donations" ) );
}

void Application::openReleaseNotes()
{
    QDesktopServices::openUrl( m_updateClient->notesUrl() );
}

void Application::openDownloads()
{
    QDesktopServices::openUrl( m_updateClient->downloadUrl() );
}

static void updatePalette( QWidget* widget, const QPalette& palette )
{
    widget->setPalette( palette );
    widget->style()->polish( widget );

    foreach ( QObject* child, widget->children() ) {
        if ( child->isWidgetType() ) {
            QWidget* childWidget = (QWidget*)child;
            if ( childWidget->isTopLevel() )
                updatePalette( childWidget, application->palette() );
            else
                updatePalette( childWidget, widget->palette() );
        }
    }
}

void Application::settingsChanged()
{
    m_updateClient->setAutoUpdate( m_settings->value( "AutoUpdate" ).toBool() );

    QString oldTheme = m_theme;
    m_theme = m_settings->value( "Theme" ).toString();

    if ( !oldTheme.isEmpty() && m_theme != oldTheme )
        updateTheme();
}

void Application::updateTheme()
{
    initializePalette();

    foreach ( QWidget* widget, topLevelWidgets() )
        updatePalette( widget, palette() );

    QPixmapCache::clear();

    emit themeChanged();

    if ( m_notesSection )
        m_notesSection->setPixmap( IconLoader::pixmap( "web" ) );
    if ( m_helpSection )
        m_helpSection->setPixmap( IconLoader::pixmap( "help" ) );
    if ( m_updateSection )
        m_updateSection->setPixmap( IconLoader::pixmap( "info" ) );
    if ( m_licenseSection )
        m_licenseSection->setPixmap( IconLoader::pixmap( "license" ) );
}

QString Application::version() const
{
    return QString( "1.0" );
}

bool Application::loadTranslation( const QString& name, bool tryQtDir )
{
    QString fullName = name + "_" + QLocale().name();

    QTranslator* translator = new QTranslator( this );

    if ( translator->load( fullName, m_translationsPath ) ) {
        installTranslator( translator );
        return true;
    }

    if ( tryQtDir && translator->load( fullName, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ) ) {
        installTranslator( translator );
        return true;
    }

    delete translator;
    return false;
}

void Application::initializeDefaultPaths()
{
    QString appPath = applicationDirPath();

    m_translationsPath = QDir::cleanPath( appPath + "/../translations" );

    setApplicationName( "Saladin" );
    m_dataPath = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
    m_cachePath = QStandardPaths::writableLocation( QStandardPaths::AppLocalDataLocation ) + "/cache";

    m_tempPath = QDir::tempPath() + "/Saladin";
}

void Application::initializeSettings()
{
    if ( !m_settings->contains( "Directory1" ) )
        m_settings->setValue( "Directory1", QVariant::fromValue( ShellFolder::defaultFolder() ) );
    if ( !m_settings->contains( "Directory2" ) )
        m_settings->setValue( "Directory2", QVariant::fromValue( ShellFolder::defaultFolder() ) );

    if ( !m_settings->contains( "RememberDirectories" ) )
        m_settings->setValue( "RememberDirectories", false );

    if ( !m_settings->contains( "HomeDirectory1" ) ) {
        if ( !m_settings->value( "RememberDirectories" ).toBool() )
            m_settings->setValue( "HomeDirectory1", m_settings->value( "Directory1" ) );
        else
            m_settings->setValue( "HomeDirectory1", QVariant::fromValue( ShellFolder::defaultFolder() ) );
    }
    if ( !m_settings->contains( "HomeDirectory2" ) ) {
        if ( !m_settings->value( "RememberDirectories" ).toBool() )
            m_settings->setValue( "HomeDirectory2", m_settings->value( "Directory2" ) );
        else
            m_settings->setValue( "HomeDirectory2", QVariant::fromValue( ShellFolder::defaultFolder() ) );
    }

    if ( !m_settings->contains( "BinaryFont" ) )
        m_settings->setValue( "BinaryFont", "Courier New" );
    if ( !m_settings->contains( "BinaryFontSize" ) )
        m_settings->setValue( "BinaryFontSize", 10 );

    if ( !m_settings->contains( "Theme" ) )
        m_settings->setValue( "Theme", "classic" );

    if ( !m_settings->contains( "HideToolStrip" ) )
        m_settings->setValue( "HideToolStrip", false );

    if ( !m_settings->contains( "TextFont" ) )
        m_settings->setValue( "TextFont", "Courier New" );
    if ( !m_settings->contains( "TextFontSize" ) )
        m_settings->setValue( "TextFontSize", 10 );

    if ( !m_settings->contains( "ConfirmDnd" ) )
        m_settings->setValue( "ConfirmDnd", true );

    if ( !m_settings->contains( "AutoUpdate" ) )
        m_settings->setValue( "AutoUpdate", true );

    if ( !m_settings->contains( "InternalViewer" ) )
        m_settings->setValue( "InternalViewer", true );

    if ( !m_settings->contains( "EditorTool" ) ) {
        wchar_t buffer[ MAX_PATH ];
        if ( SHGetSpecialFolderPath( 0, buffer, CSIDL_SYSTEM, FALSE ) )
            m_settings->setValue( "EditorTool", QString::fromWCharArray( buffer ) + "\\notepad.exe" );
    }

    if ( !m_settings->contains( "ConsoleTool" ) ) {
        wchar_t buffer[ MAX_PATH ];
        if ( SHGetSpecialFolderPath( 0, buffer, CSIDL_SYSTEM, FALSE ) )
            m_settings->setValue( "ConsoleTool", QString::fromWCharArray( buffer ) + "\\cmd.exe" );
    }
}

void Application::initializePalette()
{
    QString theme = m_settings->value( "Theme" ).toString();

    if ( theme == QLatin1String( "white" ) ) {
        QPalette pal( Qt::white );
        pal.setColor( QPalette::Window, style()->standardPalette().color( QPalette::Window ) );
        pal.setColor( QPalette::Highlight, palette().color( QPalette::Highlight ) );
        setPalette( pal );
    } else if ( theme == QLatin1String( "dark" ) ) {
        QPalette pal( QColor( 26, 26, 26 ) );
        pal.setColor( QPalette::Base, Qt::black );
        pal.setColor( QPalette::AlternateBase, QColor( 26, 26, 26 ) );
        pal.setColor( QPalette::Text, Qt::black );
        pal.setColor( QPalette::ButtonText, Qt::black );
        pal.setColor( QPalette::Highlight, palette().color( QPalette::Highlight ) );
        pal.setColor( QPalette::Link, QColor( 51, 153, 255 ) );
        setPalette( pal );
    } else {
        setPalette( style()->standardPalette() );
    }
}

QString Application::iconsPath() const
{
    QString theme = m_settings->value( "Theme" ).toString();

    if ( theme == QLatin1String( "dark" ) )
        return ":/icons/dark";
    else
        return ":/icons";
}

QString Application::locateDataFile( const QString& name )
{
    QString path = m_dataPath + '/' + name;

    checkAccess( path );

    return path;
}

QString Application::locateCacheFile( const QString& name )
{
    QString path = m_cachePath + '/' + name;

    checkAccess( path );

    return path;
}

QString Application::locateTempFile( const QString& name )
{
    QString path = m_tempPath + '/' + name;

    checkAccess( path );

    return path;
}

bool Application::checkAccess( const QString& path )
{
    QFileInfo fileInfo( path );
    if ( fileInfo.exists() )
        return fileInfo.isReadable();

    QDir dir = fileInfo.absoluteDir();
    if ( dir.exists() )
        return dir.isReadable();

    return dir.mkpath( dir.absolutePath() );
}

void Application::setBookmarks( const QList<Bookmark>& bookmarks )
{
    m_bookmarks = bookmarks;

    saveBookmarks();
}

void Application::addBookmark( const Bookmark& bookmark )
{
    m_bookmarks.append( bookmark );

    saveBookmarks();
}

void Application::loadBookmarks()
{
    DataSerializer serializer( locateDataFile( "bookmarks.dat" ) );

    if ( !serializer.openForReading() )
        return;

    serializer.stream() >> m_bookmarks;
}

void Application::saveBookmarks()
{
    DataSerializer serializer( locateDataFile( "bookmarks.dat" ) );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream() << m_bookmarks;
}
