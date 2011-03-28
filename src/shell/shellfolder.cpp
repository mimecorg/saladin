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

#include "shellfolder.h"
#include "shellfolder_p.h"
#include "shellitem_p.h"
#include "shellpidl_p.h"
#include "iconcache_p.h"
#include "changenotifywatcher_p.h"

#include <shlwapi.h>
#include <ntquery.h>

ShellFolder::ShellFolder( const QString& path, QWidget* parent ) : QObject( parent ),
    d( new ShellFolderPrivate() )
{
    d->q = this;

    HRESULT hr = SHParseDisplayName( (PCWSTR)path.utf16(), NULL, &d->m_pidl, 0, NULL );

    if ( SUCCEEDED( hr ) ) {
        hr = SHBindToObject( NULL, d->m_pidl, NULL, IID_PPV_ARGS( &d->m_folder ) );

        if ( SUCCEEDED( hr ) )
            d->readProperties();
    }
}

ShellFolder::ShellFolder( const ShellPidl& pidl, QWidget* parent ) : QObject( parent ),
    d( new ShellFolderPrivate() )
{
    d->q = this;

    d->m_pidl = ILClone( pidl.d->pidl() );

    HRESULT hr = SHBindToObject( NULL, d->m_pidl, NULL, IID_PPV_ARGS( &d->m_folder ) );

    if ( SUCCEEDED( hr ) )
        d->readProperties();
}

ShellFolder::ShellFolder( IShellFolder* folder, QWidget* parent ) : QObject( parent ),
    d( new ShellFolderPrivate() )
{
    d->q = this;
    d->m_folder = folder;

    HRESULT hr = SHGetIDListFromObject( folder, &d->m_pidl );

    if ( SUCCEEDED( hr ) )
        d->readProperties();
}

ShellFolder::~ShellFolder()
{
    delete d;
}

ShellFolderPrivate::ShellFolderPrivate() :
    q( NULL ),
    m_hasParent( false )
{
}

ShellFolderPrivate::~ShellFolderPrivate()
{
}

void ShellFolderPrivate::readProperties()
{
    wchar_t* name;
    HRESULT hr = SHGetNameFromIDList( m_pidl, SIGDN_DESKTOPABSOLUTEEDITING, &name );

    if ( SUCCEEDED( hr ) ) {
        m_path = QString::fromWCharArray( name );
        CoTaskMemFree( name );
    }

    if ( m_path.startsWith( QLatin1String( "ftp://" ), Qt::CaseInsensitive ) ) {
        QUrl url = QUrl( m_path );
        m_path = url.toString( QUrl::RemoveUserInfo );
        m_user = url.userName();
        m_password = url.password();
    }

    m_hasParent = true;

    LPITEMIDLIST pidl;
    hr = SHGetSpecialFolderLocation( q->parent()->effectiveWinId(), CSIDL_DESKTOP, &pidl );

    if ( SUCCEEDED( hr ) ) {
        if ( ILIsEqual( pidl, m_pidl ) )
            m_hasParent = false;

        CoTaskMemFree( pidl );
    }

    const int rootFolders[ 3 ] = { CSIDL_DRIVES, CSIDL_NETWORK, CSIDL_INTERNET };

    for ( int i = 0; i < 3 && m_hasParent; i++ ) {
        LPITEMIDLIST pidl;
        hr = SHGetSpecialFolderLocation( q->parent()->effectiveWinId(), rootFolders[ i ], &pidl );

        if ( SUCCEEDED( hr ) ) {
            if ( ILIsParent( pidl, m_pidl, true ) )
                m_hasParent = false;

            CoTaskMemFree( pidl );
        }
    }
}

bool ShellFolder::isValid() const
{
    return d->m_folder != NULL;
}

QString ShellFolder::path() const
{
    return d->m_path;
}

bool ShellFolder::hasParent() const
{
    return d->m_hasParent;
}

ShellItem::Attributes ShellFolder::attributes()
{
    ShellItem::Attributes result = 0;

    ShellItem item;
    ShellFolder* parentFolder = d->createParentFolder( item );

    if ( parentFolder ) {
        parentFolder->d->readItemProperties( item );
        result = item.d->m_attributes;

        delete parentFolder;
    }

    return result;
}

QString ShellFolder::name()
{
    QString result;

    wchar_t* name;
    HRESULT hr = SHGetNameFromIDList( d->m_pidl, SIGDN_PARENTRELATIVEEDITING, &name );

    if ( SUCCEEDED( hr ) ) {
        result = QString::fromWCharArray( name );
        CoTaskMemFree( name );
    }

    return result;
}

ShellPidl ShellFolder::pidl() const
{
    ShellPidl pidl;
    pidl.d->m_data = QByteArray( (const char*)d->m_pidl, (int)ILGetSize( d->m_pidl ) );
    pidl.d->m_path = d->m_path;
    return pidl;
}

QString ShellFolder::user() const
{
    return d->m_user;
}

QString ShellFolder::password() const
{
    return d->m_password;
}

QList<ShellItem> ShellFolder::listItems( Flags flags )
{
    QList<ShellItem> result;

    SHCONTF enumFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
    if ( flags.testFlag( IncludeHidden ) )
        enumFlags |= SHCONTF_INCLUDEHIDDEN;

    IEnumIDList* enumerator;
    HRESULT hr = d->m_folder->EnumObjects( parent()->effectiveWinId(), enumFlags, &enumerator );

    if ( SUCCEEDED( hr ) && enumerator ) {
        LPITEMIDLIST pidl;
        while ( enumerator->Next( 1, &pidl, NULL ) == S_OK )
            result.append( d->makeItem( pidl ) );

        enumerator->Release();
    }

    return result;
}

ShellItem ShellFolderPrivate::makeItem( LPITEMIDLIST pidl )
{
    ShellItem item;
    item.d->m_pidl = pidl;
    readItemProperties( item );
    return item;
}

ShellItem ShellFolderPrivate::makeNotifyItem( LPITEMIDLIST pidl )
{
    ShellItem item;
    item.d->m_pidl = ILClone( pidl );
    return item;
}

ShellItem ShellFolderPrivate::makeRealNotifyItem( LPITEMIDLIST pidl )
{
    ShellItem item;

    HRESULT hr = SHGetRealIDL( m_folder, pidl, &item.d->m_pidl );

    if ( SUCCEEDED( hr ) )
        readItemProperties( item );

    return item;
}

static QDateTime systemTimeToQDateTime( const SYSTEMTIME* systemTime )
{
    SYSTEMTIME localTime;
    SystemTimeToTzSpecificLocalTime( NULL, systemTime, &localTime );

    QDate date( localTime.wYear, localTime.wMonth, localTime.wDay );
    QTime time( localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds );

    return QDateTime( date, time, Qt::LocalTime );
}

static QDateTime fileTimeToQDateTime( const FILETIME* fileTime )
{
    SYSTEMTIME systemTime;
    FileTimeToSystemTime( fileTime, &systemTime );

    return systemTimeToQDateTime( &systemTime );
}

static QDateTime variantTimeToQDateTime( double vtime )
{
    SYSTEMTIME systemTime;
    VariantTimeToSystemTime( vtime, &systemTime );

    return systemTimeToQDateTime( &systemTime );
}

static ShellItem::Attributes fileAttributesToAttributes( DWORD fileAttributes )
{
    ShellItem::Attributes attributes = 0;

    if ( fileAttributes & FILE_ATTRIBUTE_READONLY )
        attributes |= ShellItem::ReadOnly;
    if ( fileAttributes & FILE_ATTRIBUTE_HIDDEN )
        attributes |= ShellItem::Hidden;
    if ( fileAttributes & FILE_ATTRIBUTE_SYSTEM )
        attributes |= ShellItem::System;
    if ( fileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        attributes |= ShellItem::Directory;
    if ( fileAttributes & FILE_ATTRIBUTE_ARCHIVE )
        attributes |= ShellItem::Archive;
    if ( fileAttributes & FILE_ATTRIBUTE_COMPRESSED )
        attributes |= ShellItem::Compressed;
    if ( fileAttributes & FILE_ATTRIBUTE_ENCRYPTED )
        attributes |= ShellItem::Encrypted;
    if ( fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
        attributes |= ShellItem::ReparsePoint;

    return attributes;
}

void ShellFolderPrivate::readItemProperties( ShellItem& item )
{
    item.d->m_name = displayName( item.d->m_pidl, SHGDN_INFOLDER );

    ShellItem::Attributes attributes = 0;
    ShellItem::State state = 0;

    ULONG shellAttributes = SFGAO_FOLDER | SFGAO_STREAM | SFGAO_FILESYSTEM | SFGAO_CANRENAME;
    HRESULT hr = m_folder->GetAttributesOf( 1, (LPCITEMIDLIST*)&item.d->m_pidl, &shellAttributes );

    if ( SUCCEEDED( hr ) ) {
        if ( shellAttributes & SFGAO_FOLDER )
            attributes |= ShellItem::Folder;
        if ( shellAttributes & SFGAO_STREAM )
            attributes |= ShellItem::Stream;
        if ( shellAttributes & SFGAO_FILESYSTEM )
            attributes |= ShellItem::FileSystem;
        if ( shellAttributes & SFGAO_CANRENAME )
            attributes |= ShellItem::CanRename;
    }

    WIN32_FIND_DATA data;
    hr = SHGetDataFromIDList( m_folder, item.d->m_pidl, SHGDFIL_FINDDATA, &data, sizeof( data ) );

    if ( SUCCEEDED( hr ) ) {
        attributes |= fileAttributesToAttributes( data.dwFileAttributes );

        item.d->m_name = QString::fromWCharArray( data.cFileName );

        item.d->m_size = (qint64)data.nFileSizeHigh << 32 | data.nFileSizeLow;

        item.d->m_modified = fileTimeToQDateTime( &data.ftLastWriteTime );

        state |= ShellItem::HasProperties;
    } else {
        if ( attributes.testFlag( ShellItem::Folder ) && !attributes.testFlag( ShellItem::Stream ) )
            attributes |= ShellItem::Directory;

        IShellFolder2* folder2;
        hr = m_folder->QueryInterface( IID_PPV_ARGS( &folder2 ) );

        if ( SUCCEEDED( hr ) ) {
            SHCOLUMNID shcolumnid;
            shcolumnid.fmtid = FMTID_Storage;
            shcolumnid.pid = PID_STG_WRITETIME;

            VARIANT variant = { 0 };

            hr = folder2->GetDetailsEx( item.d->m_pidl, &shcolumnid, &variant );

            if ( SUCCEEDED( hr ) ) {
                item.d->m_modified = variantTimeToQDateTime( variant.date );

                shcolumnid.pid = PID_STG_ATTRIBUTES;
                hr = folder2->GetDetailsEx( item.d->m_pidl, &shcolumnid, &variant );

                if ( SUCCEEDED( hr ) )
                    attributes |= fileAttributesToAttributes( variant.uintVal );

                if ( !attributes.testFlag( ShellItem::Directory ) ) {
                    shcolumnid.pid = PID_STG_SIZE;
                    hr = folder2->GetDetailsEx( item.d->m_pidl, &shcolumnid, &variant );

                    if ( SUCCEEDED( hr ) )
                        item.d->m_size = variant.llVal;
                }

                state |= ShellItem::HasProperties;
            }

            folder2->Release();
        }
    }

    item.d->m_attributes = attributes;
    item.d->m_state = state;

    QPixmap icon;

    if ( attributes.testFlag( ShellItem::Directory ) ) {
        icon = IconCache::directoryIcon();
    } else {
        QString extension;
        int pos = item.d->m_name.lastIndexOf( '.' );
        if ( pos > 0 )
            extension = item.d->m_name.mid( pos + 1 ).toLower();

        if ( !extension.isEmpty() )
            icon = IconCache::loadFileIcon( extension );

        if ( icon.isNull() )
            icon = IconCache::fileIcon();
    }

    item.d->m_icon = icon;
}

bool ShellFolder::extractIcon( ShellItem& item )
{
    bool result = false;

    QPixmap icon = d->extractIcon( item.d->m_pidl );

    if ( !icon.isNull() ) {
        item.d->m_icon = icon;
        result = true;
    }

    item.d->m_state |= ShellItem::HasExtractedIcon;

    return result;
}

qint64 ShellFolderPrivate::calculateSize( IShellFolder* parentFolder, LPITEMIDLIST pidl )
{
    qint64 size = 0;

    IShellFolder* folder;
    HRESULT hr = SHBindToObject( parentFolder, pidl, NULL, IID_PPV_ARGS( &folder ) );

    if ( SUCCEEDED( hr ) ) {
        IEnumIDList* enumerator;
        hr = folder->EnumObjects( q->parent()->effectiveWinId(), SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &enumerator );

        if ( SUCCEEDED( hr ) && enumerator ) {
            LPITEMIDLIST itemPidl;
            while ( enumerator->Next( 1, &itemPidl, NULL ) == S_OK ) {
                WIN32_FIND_DATA data;
                hr = SHGetDataFromIDList( folder, itemPidl, SHGDFIL_FINDDATA, &data, sizeof( data ) );

                if ( SUCCEEDED( hr ) ) {
                    size += (qint64)data.nFileSizeHigh << 32 | data.nFileSizeLow;

                    if ( ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && !( data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) )
                        size += calculateSize( folder, itemPidl );
                }

                CoTaskMemFree( itemPidl );
            }

            enumerator->Release();
        }

        folder->Release();
    }

    return size;
}

bool ShellFolder::calculateSize( ShellItem& item )
{
    if ( item.d->m_state.testFlag( ShellItem::HasCalculatedSize ) || !item.d->m_state.testFlag( ShellItem::HasProperties ) )
        return false;

    if ( !item.d->m_attributes.testFlag( ShellItem::Directory ) )
        return false;

    item.d->m_size = d->calculateSize( d->m_folder, item.d->m_pidl );
    item.d->m_state |= ShellItem::HasCalculatedSize;

    return true;
}

bool ShellFolder::setItemName( ShellItem& item, const QString& name )
{
    bool result = false;

    LPITEMIDLIST pidlNew = NULL;
    HRESULT hr = d->m_folder->SetNameOf( parent()->effectiveWinId(), item.d->m_pidl, (LPCWSTR)name.utf16(), SHGDN_INFOLDER | SHGDN_FORPARSING, &pidlNew );

    if ( SUCCEEDED( hr ) ) {
        if ( pidlNew ) {
            LPITEMIDLIST absolutePidl = ILCombine( d->m_pidl, item.d->m_pidl );
            LPITEMIDLIST absolutePidlNew = ILCombine( d->m_pidl, pidlNew );

            SHChangeNotify( SHCNE_RENAMEFOLDER, SHCNF_IDLIST, absolutePidl, absolutePidlNew );

            CoTaskMemFree( absolutePidl );
            CoTaskMemFree( absolutePidlNew );

            CoTaskMemFree( item.d->m_pidl );
            item.d->m_pidl = pidlNew;
        }

        item.d->m_name = name;
        item.d->m_state &= ~ShellItem::HasExtractedIcon;

        result = true;
    }

    return result;
}

QString ShellFolder::itemPath( const ShellItem& item )
{
    QString result;

    LPITEMIDLIST absolutePidl = ILCombine( d->m_pidl, item.d->m_pidl );

    wchar_t buffer[ 1024 ];
    if ( SHGetPathFromIDListEx( absolutePidl, buffer, 1024, GPFIDL_DEFAULT ) )
        result = QString::fromWCharArray( buffer );

    CoTaskMemFree( absolutePidl );

    return result;
}

bool ShellFolder::isEqual( const ShellFolder* other ) const
{
    return ILIsEqual( d->m_pidl, other->d->m_pidl );
}

ShellFolder* ShellFolder::clone()
{
    d->m_folder->AddRef();
    return new ShellFolder( d->m_folder, parent() );
}

ShellFolder* ShellFolder::openFolder( const ShellItem& item )
{
    if ( !item.attributes().testFlag( ShellItem::Folder ) )
        return NULL;

    ShellFolder* result = NULL;

    IShellFolder* folder;
    HRESULT hr = SHBindToObject( d->m_folder, item.d->m_pidl, NULL, IID_PPV_ARGS( &folder ) );

    if ( SUCCEEDED( hr ) )
        result = new ShellFolder( folder, parent() );

    return result;
}

ShellFolder* ShellFolder::parentFolder( ShellItem& item )
{
    if ( !hasParent() )
        return NULL;

    return d->createParentFolder( item );
}

ShellFolder* ShellFolderPrivate::createParentFolder( ShellItem& item )
{
    ShellFolder* result = NULL;

    IShellFolder* folder;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToParent( m_pidl, IID_PPV_ARGS( &folder ), &pidlLast );

    if ( SUCCEEDED( hr ) ) {
        item.d->m_pidl = ILClone( pidlLast );

        result = new ShellFolder( folder, q->parent() );
    }

    return result;
}

ShellFolder* ShellFolder::rootFolder()
{
    ShellFolder* result = NULL;

    for ( const ShellFolder* folder = this; folder && folder->hasParent(); folder = result ) {
        ShellItem item;
        result = folder->d->createParentFolder( item );

        if ( folder != this )
            delete folder;
    }

    return result;
}

static int CALLBACK ShellFolderBrowseCallbackProc( HWND hwnd, UINT msg, LPARAM /*lparam*/, LPARAM data )
{
    if ( msg == BFFM_INITIALIZED )
        SendMessage( hwnd, BFFM_SETSELECTION, false, data );
    return 0;
}

ShellFolder* ShellFolder::browseFolder()
{
    ShellFolder* result = NULL;

    wchar_t buffer[ MAX_PATH ];

    QString title = tr( "Select the folder to open:" );

    BROWSEINFO info = { 0 };
    info.hwndOwner = parent()->effectiveWinId();
    info.pidlRoot = NULL;
    info.pszDisplayName = buffer;
    info.lpszTitle = title.utf16();
    info.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    info.lpfn = ShellFolderBrowseCallbackProc;
    info.lParam = (LPARAM)d->m_pidl;

    LPITEMIDLIST pidl = SHBrowseForFolder( &info );

    if ( pidl ) {
        IShellFolder* folder;
        HRESULT hr = SHBindToObject( NULL, pidl, NULL, IID_PPV_ARGS( &folder ) );

        if ( SUCCEEDED( hr ) )
            result = new ShellFolder( folder, parent() );

        CoTaskMemFree( pidl );
    }

    return result;
}

ShellPidl ShellFolder::browseFolder( QWidget* parent, const QString& title, const ShellPidl& startPidl )
{
    ShellPidl result;

    wchar_t buffer[ MAX_PATH ];

    BROWSEINFO info = { 0 };
    info.hwndOwner = parent->effectiveWinId();
    info.pidlRoot = NULL;
    info.pszDisplayName = buffer;
    info.lpszTitle = title.utf16();
    info.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    info.lpfn = ShellFolderBrowseCallbackProc;
    info.lParam = (LPARAM)startPidl.d->pidl();

    LPITEMIDLIST pidl = SHBrowseForFolder( &info );

    if ( pidl ) {
        result.d->m_data = QByteArray( (const char*)pidl, (int)ILGetSize( pidl ) );

        wchar_t realPath[ 1024 ];
        if ( SHGetPathFromIDListEx( pidl, realPath, 1024, GPFIDL_DEFAULT ) )
            result.d->m_path = QString::fromWCharArray( realPath );
        else
            result.d->m_path = QString::fromWCharArray( buffer );

        CoTaskMemFree( pidl );
    }

    return result;
}

ShellPidl ShellFolder::defaultFolder()
{
    ShellPidl result;

    result.d->m_path = QDir::toNativeSeparators( QDir::rootPath() );

    LPITEMIDLIST pidl;
    HRESULT hr = SHParseDisplayName( (PCWSTR)result.d->m_path.utf16(), NULL, &pidl, 0, NULL );

    if ( SUCCEEDED( hr ) ) {
        result.d->m_data = QByteArray( (const char*)pidl, (int)ILGetSize( pidl ) );

        CoTaskMemFree( pidl );
    }

    return result;
}

bool ShellFolder::canCreateFolder()
{
    bool result = false;

    ITransferDestination* destination;
    HRESULT hr = d->m_folder->CreateViewObject( parent()->effectiveWinId(), IID_PPV_ARGS( &destination ) );

    if ( SUCCEEDED( hr ) ) {
        destination->Release();
        result = true;
    }

    return result;
}

bool ShellFolder::createFolder( const QString& name )
{
    bool result = false;

    IFileOperation* fileOperation;
    HRESULT hr = CoCreateInstance( CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS( &fileOperation ) );

    if ( SUCCEEDED( hr ) ) {
        fileOperation->SetOwnerWindow( parent()->effectiveWinId() );

        DWORD operationFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOFX_NOMINIMIZEBOX;
        fileOperation->SetOperationFlags( operationFlags );

        IShellItem* item;
        hr = SHCreateItemFromIDList( d->m_pidl, IID_PPV_ARGS( &item ) );

        if ( SUCCEEDED( hr ) ) {
            hr = fileOperation->NewItem( item, FILE_ATTRIBUTE_DIRECTORY, name.utf16(), NULL, NULL );

            if ( SUCCEEDED( hr ) ) {
                hr = fileOperation->PerformOperations();

                if ( SUCCEEDED( hr ) )
                    result = true;
            }

            item->Release();
        }

        fileOperation->Release();
    }

    return result;
}

bool ShellFolder::createFile( const QString& name, const char* data /*= NULL*/, int size /*= 0*/ )
{
    bool result = false;

    QString path = d->m_path + QLatin1Char( '\\' ) + name;

    DWORD creation = ( data && size > 0 ) ? CREATE_ALWAYS : OPEN_ALWAYS;
    HANDLE file = CreateFile( path.utf16(), GENERIC_WRITE, FILE_SHARE_READ, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( file != INVALID_HANDLE_VALUE ) {
        if ( data && size > 0 ) {
            DWORD written;
            result = WriteFile( file, data, size, &written, NULL );
        } else {
            result = true;
        }

        CloseHandle( file );
    }

    return result;
}

ShellItem ShellFolder::childItem( const QString& name )
{
    ShellItem result;

    HRESULT hr = d->m_folder->ParseDisplayName( parent()->effectiveWinId(), NULL, (LPWSTR)name.utf16(), NULL, &result.d->m_pidl, NULL );

    if ( SUCCEEDED( hr ) )
        d->readItemProperties( result );

    return result;
}

bool ShellFolder::executeItem( const ShellItem& item )
{
    LPITEMIDLIST absolutePidl = ILCombine( d->m_pidl, item.d->m_pidl );

    SHELLEXECUTEINFO info = { 0 };
    info.cbSize = sizeof( info );
    info.fMask = SEE_MASK_IDLIST;
    info.hwnd = parent()->effectiveWinId();
    info.lpVerb = NULL;
    info.lpParameters = NULL;
    info.lpDirectory = d->m_path.utf16();
    info.nShow = SW_SHOWNORMAL;
    info.lpIDList = absolutePidl;

    bool result = ShellExecuteEx( &info );

    CoTaskMemFree( absolutePidl );

    return result;
}

ShellSelection::MenuCommand ShellFolder::showContextMenu( const QPoint& pos, ShellSelection::Flags flags )
{
    ShellSelection::MenuCommand result = ShellSelection::NoCommand;

    ShellItem item;
    ShellFolder* parentFolder = d->createParentFolder( item );

    if ( parentFolder ) {
        QList<ShellItem> items;
        items.append( item );

        ShellSelection selection( parentFolder, items, parent() );
        result = selection.showContextMenu( pos, flags );

        delete parentFolder;
    }

    return result;
}

bool ShellFolder::invokeCommand( const char* verb )
{
    bool result = false;

    ShellItem item;
    ShellFolder* parentFolder = d->createParentFolder( item );

    if ( parentFolder ) {
        QList<ShellItem> items;
        items.append( item );

        ShellSelection selection( parentFolder, items, parent() );
        result = selection.invokeCommand( verb );

        delete parentFolder;
    }

    return result;
}

bool ShellFolder::search()
{
    return SHFindFiles( d->m_pidl, NULL );
}

bool ShellFolder::explore()
{
    SHELLEXECUTEINFO info = { 0 };
    info.cbSize = sizeof( info );
    info.fMask = SEE_MASK_IDLIST;
    info.hwnd = parent()->effectiveWinId();
    info.lpVerb = NULL;
    info.lpParameters = NULL;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOWNORMAL;
    info.lpIDList = d->m_pidl;

    return ShellExecuteEx( &info );
}

QString ShellFolder::toolTip( const ShellItem& item )
{
    QString result;

    IQueryInfo* queryInfo;
    HRESULT hr = d->m_folder->GetUIObjectOf( parent()->effectiveWinId(), 1, (LPCITEMIDLIST*)&item.d->m_pidl, IID_IQueryInfo, NULL, (void**)&queryInfo );

    if ( SUCCEEDED( hr ) ) {
        wchar_t* tip;
        hr = queryInfo->GetInfoTip( QITIPF_DEFAULT, &tip );

        if ( SUCCEEDED( hr ) ) {
            result = QString::fromWCharArray( tip );

            CoTaskMemFree( tip );
        }
        queryInfo->Release();
    }

    return result;
}

bool ShellFolder::startWatching()
{
    ChangeNotifyWatcher* watcher = new ChangeNotifyWatcher( d->m_pidl, SHCNE_DISKEVENTS, this );

    if ( watcher->isValid() ) {
        connect( watcher, SIGNAL( changeNotify( int, void*, void* ) ), this, SLOT( changeNotify( int, void*, void* ) ) );
        return true;
    }

    return false;
}

void ShellFolder::changeNotify( int eventType, void* arg1, void* arg2 )
{
    if ( eventType == SHCNE_UPDATEDIR && ILIsEqual( d->m_pidl, (LPITEMIDLIST)arg1 ) ) {
        emit folderUpdated();
        return;
    }

    if ( eventType == SHCNE_CREATE || eventType == SHCNE_MKDIR ) {
        LPITEMIDLIST pidl = ILFindChild( d->m_pidl, (LPITEMIDLIST)arg1 );
        if ( pidl && !ILIsEmpty( pidl ) ) {
            ShellItem item = d->makeRealNotifyItem( pidl );
            if ( item.isValid() )
                emit itemChanged( ItemChange( ItemChange::ItemAdded, item ) );
            return;
        }
    }

    if ( eventType == SHCNE_DELETE || eventType == SHCNE_RMDIR ) {
        LPITEMIDLIST pidl = ILFindChild( d->m_pidl, (LPITEMIDLIST)arg1 );
        if ( pidl && !ILIsEmpty( pidl ) ) {
            ShellItem item = d->makeNotifyItem( pidl );
            emit itemChanged( ItemChange( ItemChange::ItemRemoved, item ) );
            return;
        }
    }

    if ( eventType == SHCNE_UPDATEITEM || eventType == SHCNE_UPDATEDIR ) {
        LPITEMIDLIST pidl = ILFindChild( d->m_pidl, (LPITEMIDLIST)arg1 );
        if ( pidl && !ILIsEmpty( pidl ) ) {
            ShellItem item = d->makeRealNotifyItem( pidl );
            if ( item.isValid() )
                emit itemChanged( ItemChange( ItemChange::ItemUpdated, item ) );
            return;
        }
    }

    if ( eventType == SHCNE_RENAMEITEM || eventType == SHCNE_RENAMEFOLDER ) {
        LPITEMIDLIST fromPidl = ILFindChild( d->m_pidl, (LPITEMIDLIST)arg1 );
        LPITEMIDLIST toPidl = ILFindChild( d->m_pidl, (LPITEMIDLIST)arg2 );
        if ( fromPidl && !ILIsEmpty( fromPidl ) && toPidl && !ILIsEmpty( toPidl ) ) {
            ShellItem fromItem = d->makeNotifyItem( fromPidl );
            ShellItem toItem = d->makeRealNotifyItem( toPidl );
            if ( toItem.isValid() )
                emit itemChanged( ItemChange( ItemChange::ItemRenamed, fromItem, toItem ) );
            return;
        }
    }
}

ItemChange::ItemChange( ItemChange::Type type, const ShellItem& item1, const ShellItem& item2 /*= ShellItem()*/ ) :
    m_type( type ),
    m_item1( item1 ),
    m_item2( item2 )
{
}

ItemChange::~ItemChange()
{
}

ItemChange::ItemChange( const ItemChange& other ) :
    m_type( other.m_type ),
    m_item1( other.m_item1 ),
    m_item2( other.m_item2 )
{
}

ItemChange& ItemChange::operator =( const ItemChange& other )
{
    m_type = other.m_type;
    m_item1 = other.m_item1;
    m_item2 = other.m_item2;
    return *this;
}
