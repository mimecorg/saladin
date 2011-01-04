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

#include "application.h"
#include "mainwindow.h"

#include "utils/localsettings.h"
#include "utils/dataserializer.h"
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

    m_mainWindow = new MainWindow();
    m_mainWindow->initialize();
    m_mainWindow->show();
    m_mainWindow->openDirectories();
}

Application::~Application()
{
    delete m_mainWindow;
    m_mainWindow = NULL;

    delete m_settings;
    m_settings = NULL;
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

void Application::about()
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

    QString message;
    message += "<h3>" + tr( "Saladin %1" ).arg( version() ) + "</h3>";
    message += "<p>" + tr( "Dual-pane file manager for Windows." ) + "</p>";
    message += "<p>" + tr( "Built on %1 in %2-bit %3 mode." ).arg( compiled.toString( "yyyy-MM-dd HH:mm" ), configBits, configMode );
    message += "<br>" + tr( "Using Qt %1 (%2 linking) and Windows Shell %3." ).arg( qtVersion, configLink, shellVersion ) + "</p>";
    message += "<p>" + tr( "This program is free software: you can redistribute it and/or modify"
        " it under the terms of the GNU General Public License as published by"
        " the Free Software Foundation, either version 3 of the License, or"
        " (at your option) any later version." ) + "</p>";
    message += "<p>" + trUtf8( "Copyright (C) 2010 Michał Męciński" ) + "</p>";

    QMessageBox::about( activeWindow(), tr( "About Saladin" ), message );
}

QString Application::version() const
{
    return QString( "0.1" );
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

    if ( !m_settings->contains( "EditorTool" ) ) {
        wchar_t buffer[ MAX_PATH ];
        if ( SHGetSpecialFolderPath( 0, buffer, CSIDL_WINDOWS, FALSE ) )
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
