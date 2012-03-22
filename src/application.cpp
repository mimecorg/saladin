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

    setStyle( "XmlUi::WindowsStyle" );

    setWindowIcon( IconLoader::icon( "saladin" ) );

    mainWindow = new MainWindow();

    QNetworkProxyFactory::setUseSystemConfiguration( true );

    QNetworkAccessManager* manager = new QNetworkAccessManager( this );

    m_updateClient = new UpdateClient( "saladin", version(), manager );

    connect( m_updateClient, SIGNAL( stateChanged() ), this, SLOT( showUpdateState() ) );

    settingsChanged();

    connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    if ( m_settings->value( "LastVersion" ).toString() != version() ) {
        m_settings->setValue( "LastVersion", version() );
        about();
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

extern "C" IMAGE_DOS_HEADER __ImageBase;

QString Application::technicalInformation()
{
#if defined( Q_OS_WIN64 )
    QString configBits = "64";
#else
    QString configBits = "32";
#endif
#if defined ( QT_DEBUG )
    QString configMode = "debug";
#else
    QString configMode = "release";
#endif
#if defined( QT_DLL )
    QString configLink = "dynamic";
#else
    QString configLink = "static";
#endif

    QString qtVersion = qVersion();

    QString shellVersion;
    DLLVERSIONINFO2 info = { 0 };
    info.info1.cbSize = sizeof( info );
    DLLGETVERSIONPROC dllGetVersion = (DLLGETVERSIONPROC)GetProcAddress( GetModuleHandle( L"shell32" ), "DllGetVersion" );
    if ( dllGetVersion ) {
        dllGetVersion( &info.info1 );
        shellVersion = QString( "%1.%2.%3.%4" ).arg( info.info1.dwMajorVersion ).arg( info.info1.dwMinorVersion ).arg( info.info1.dwBuildNumber ).arg( (int)( info.ullVersion & DLLVER_QFE_MASK ) );
    }

    const IMAGE_NT_HEADERS* header = (const IMAGE_NT_HEADERS*)( (char*)&__ImageBase + __ImageBase.e_lfanew );
    QDateTime compiled = QDateTime::fromTime_t( header->FileHeader.TimeDateStamp );

    QString infoMessage;
    infoMessage += "<h4>" + tr( "Technical Information" ) + "</h4>";
    infoMessage += "<p>" + tr( "Built on %1 in %2-bit %3 mode." ).arg( compiled.toString( "yyyy-MM-dd HH:mm" ), configBits, configMode );
    infoMessage += " " + tr( "Using Qt %1 (%2 linking) and Windows Shell %3." ).arg( qtVersion, configLink, shellVersion ) + "</p>";

    return infoMessage;
}

void Application::about()
{
    if ( !m_aboutBox ) {
        QString message;
        message += "<h3>" + tr( "Saladin %1" ).arg( version() ) + "</h3>";
        message += "<p>" + tr( "Dual-pane file manager for Windows." ) + "</p>";
        message += "<p>" + tr( "This program is free software: you can redistribute it and/or modify"
            " it under the terms of the GNU General Public License as published by"
            " the Free Software Foundation, either version 3 of the License, or"
            " (at your option) any later version." ) + "</p>";
        message += "<p>" + trUtf8( "Copyright (C) 2011 Michał Męciński" ) + "</p>";

        QString link = "<a href=\"http://saladin.mimec.org\">saladin.mimec.org</a>";

        QString helpMessage;
        helpMessage += "<h4>" + tr( "Help" ) + "</h4>";
        helpMessage += "<p>" + tr( "Open the Saladin Quick Guide for help." ) + "</p>";

        QString webMessage;
        webMessage += "<h4>" + tr( "Web Page" ) + "</h4>";
        webMessage += "<p>" + tr( "Visit %1 for more information about Saladin." ).arg( link ) + "</p>";

        QString donateMessage;
        donateMessage += "<h4>" + tr( "Donations" ) + "</h4>";
        donateMessage += "<p>" + tr( "If you like this program, your donation will help me dedicate more time for it, support it and implement new features." ) + "</p>";

        QString updateMessage;
        updateMessage += "<h4>" + tr( "Latest Version" ) + "</h4>";
        updateMessage += "<p>" + tr( "Automatic checking for latest version is disabled. You can enable it in program settings." ) + "</p>";

        QString infoMessage = technicalInformation();

        m_aboutBox = new AboutBox( tr( "About Saladin" ), message, mainWindow );

        AboutBoxSection* helpSection = m_aboutBox->addSection( IconLoader::pixmap( "help" ), helpMessage );

        QPushButton* helpButton = helpSection->addButton( tr( "&Quick Guide" ) );
        connect( helpButton, SIGNAL( clicked() ), this, SLOT( showQuickGuide() ) );

        m_aboutBox->addSection( IconLoader::pixmap( "web" ), webMessage );

        AboutBoxSection* donateSection = m_aboutBox->addSection( IconLoader::pixmap( "bookmark" ), donateMessage );

        QPushButton* donateButton = donateSection->addButton( tr( "&Donate" ) );
        connect( donateButton, SIGNAL( clicked() ), this, SLOT( openDonations() ) );

        delete m_updateSection;

        m_updateSection = m_aboutBox->addSection( IconLoader::pixmap( "info" ), updateMessage );

        if ( m_updateClient->autoUpdate() ) {
            showUpdateState();
        } else {
            m_updateButton = m_updateSection->addButton( tr( "&Check Now" ) );
            connect( m_updateButton, SIGNAL( clicked() ), m_updateClient, SLOT( checkUpdate() ) );
        }

        m_aboutBox->addSection( IconLoader::pixmap( "info" ), infoMessage );
    }

    m_aboutBox->show();
    m_aboutBox->activateWindow();
}

void Application::showQuickGuide()
{
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

void Application::settingsChanged()
{
    m_updateClient->setAutoUpdate( m_settings->value( "AutoUpdate" ).toBool() );
}

QString Application::version() const
{
    return QString( "0.2" );
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

    wchar_t appDataPath[ MAX_PATH ];
    if ( SHGetSpecialFolderPath( 0, appDataPath, CSIDL_APPDATA, FALSE ) )
        m_dataPath = QDir::fromNativeSeparators( QString::fromWCharArray( appDataPath ) );
    else
        m_dataPath = QDir::homePath();

    m_dataPath += QLatin1String( "/Saladin" );

    wchar_t localAppDataPath[ MAX_PATH ];
    if ( SHGetSpecialFolderPath( 0, localAppDataPath, CSIDL_LOCAL_APPDATA, FALSE ) )
        m_cachePath = QDir::fromNativeSeparators( QString::fromWCharArray( localAppDataPath ) );
    else
        m_cachePath = QDir::homePath();

    m_cachePath += QLatin1String( "/Saladin/cache" );

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

    if ( !m_settings->contains( "BinaryFont" ) )
        m_settings->setValue( "BinaryFont", "Courier New" );
    if ( !m_settings->contains( "BinaryFontSize" ) )
        m_settings->setValue( "BinaryFontSize", 10 );

    if ( !m_settings->contains( "TextFont" ) )
        m_settings->setValue( "TextFont", "Courier New" );
    if ( !m_settings->contains( "TextFontSize" ) )
        m_settings->setValue( "TextFontSize", 10 );

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
