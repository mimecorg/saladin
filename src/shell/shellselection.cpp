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

#include "shellselection.h"
#include "shellselection_p.h"
#include "shellfolder.h"
#include "shellfolder_p.h"
#include "shellitem.h"
#include "shellitem_p.h"

ShellSelection::ShellSelection( ShellFolder* folder, const QList<ShellItem>& items, QWidget* parent /*= NULL*/ ) : QObject( parent ),
    d( new ShellSelectionPrivate() )
{
    d->q = this;
    d->m_sourceFolder = folder;
    d->m_sourceItems = items;
}

ShellSelection::~ShellSelection()
{
    delete d;
}

ShellSelectionPrivate::ShellSelectionPrivate() :
    q( NULL ),
    m_sourceFolder( NULL )
{
}

ShellSelectionPrivate::~ShellSelectionPrivate()
{
}

bool ShellSelection::canTransferTo( ShellFolder* targetFolder, TransferType /*type*/ )
{
    bool result = false;

    ITransferSource* source;
    HRESULT hr = d->m_sourceFolder->d->m_folder->CreateViewObject( parent()->effectiveWinId(), IID_PPV_ARGS( &source ) );

    if ( SUCCEEDED( hr ) ) {
        source->Release();

        ITransferDestination* destination;
        hr = targetFolder->d->m_folder->CreateViewObject( parent()->effectiveWinId(), IID_PPV_ARGS( &destination ) );

        if ( SUCCEEDED( hr ) ) {
            destination->Release();

            result = true;
        }
    }

    return result;
}

bool ShellSelection::transferTo( ShellFolder* targetFolder, TransferType type, Flags flags, QStringList newNames )
{
    bool result = false;

    IFileOperation* fileOperation;
    HRESULT hr = CoCreateInstance( CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS( &fileOperation ) );

    if ( SUCCEEDED( hr ) ) {
        fileOperation->SetOwnerWindow( parent()->effectiveWinId() );

        DWORD operationFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOFX_NOMINIMIZEBOX;

        if ( flags.testFlag( ForceOverwrite ) )
            operationFlags |= FOF_NOCONFIRMATION;

        fileOperation->SetOperationFlags( operationFlags );

        bool sameTarget = targetFolder->match( d->m_sourceFolder );

        IShellItem* targetItem;
        hr = SHCreateItemFromIDList( targetFolder->d->m_pidl, IID_PPV_ARGS( &targetItem ) );

        if ( SUCCEEDED( hr ) ) {
            for ( int i = 0; i < d->m_sourceItems.count(); i++ ) {
                IShellItem* item;
                hr = SHCreateItemWithParent( NULL, d->m_sourceFolder->d->m_folder, d->m_sourceItems.at( i ).d->m_pidl, IID_PPV_ARGS( &item ) );

                if ( SUCCEEDED( hr ) ) {
                    if ( type == Copy )
                        hr = fileOperation->CopyItem( item, targetItem, newNames.at( i ).utf16(), NULL );
                    else if ( sameTarget )
                        hr = fileOperation->RenameItem( item, newNames.at( i ).utf16(), NULL );
                    else
                        hr = fileOperation->MoveItem( item, targetItem, newNames.at( i ).utf16(), NULL );

                    item->Release();
                }

                if ( FAILED( hr ) )
                    break;
            }

            if ( SUCCEEDED( hr ) ) {
                hr = fileOperation->PerformOperations();

                if ( SUCCEEDED( hr ) )
                    result = true;
            }

            targetItem->Release();
        }

        fileOperation->Release();
    }

    return result;
}

bool ShellSelection::canDragDropTo( ShellFolder* targetFolder, TransferType type )
{
    return d->dragDropHelper( targetFolder, type, false );
}

bool ShellSelection::dragDropTo( ShellFolder* targetFolder, TransferType type )
{
    return d->dragDropHelper( targetFolder, type, true );
}

bool ShellSelectionPrivate::dragDropHelper( ShellFolder* targetFolder, ShellSelection::TransferType type, bool doDrop )
{
    bool result = false;

    QVector<LPCITEMIDLIST> pidls( m_sourceItems.count() );
    for ( int i = 0; i < m_sourceItems.count(); i++ )
        pidls[ i ] = m_sourceItems.at( i ).d->m_pidl;

    IDataObject* dataObject;
    HRESULT hr = m_sourceFolder->d->m_folder->GetUIObjectOf( q->parent()->effectiveWinId(), pidls.count(), pidls.data(), IID_IDataObject, NULL, (void**)&dataObject );

    if ( SUCCEEDED( hr ) ) {
        IDropTarget* dropTarget;
        hr = targetFolder->d->m_folder->CreateViewObject( q->parent()->effectiveWinId(), IID_PPV_ARGS( &dropTarget ) );

        if ( SUCCEEDED( hr ) ) {
            POINTL point = { 0, 0 };

            DWORD transfer = ( type == ShellSelection::Move ) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;

            DWORD effect = transfer;
            hr = dropTarget->DragEnter( dataObject, MK_LBUTTON, point, &effect );

            if ( SUCCEEDED( hr ) && ( effect & transfer ) ) {
                if ( doDrop ) {
                    effect = transfer;
                    hr = dropTarget->Drop( dataObject, MK_LBUTTON, point, &effect );

                    if ( SUCCEEDED( hr ) ) {
                        SHChangeNotify( SHCNE_UPDATEDIR, SHCNF_IDLIST, targetFolder->d->m_pidl, NULL );
                        result = true;
                    }
                } else {
                    result = true;
                }
            } else {
                dropTarget->DragLeave();
            }

            dropTarget->Release();
        }
    }

    return result;
}

bool ShellSelection::deleteSelection( Flags flags )
{
    bool result = false;

    IFileOperation* fileOperation;
    HRESULT hr = CoCreateInstance( CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS( &fileOperation ) );

    if ( SUCCEEDED( hr ) ) {
        fileOperation->SetOwnerWindow( parent()->effectiveWinId() );

        DWORD operationFlags = FOF_NOCONFIRMMKDIR | FOFX_NOMINIMIZEBOX;

        if ( !flags.testFlag( DeletePermanently ) )
            operationFlags |= FOF_ALLOWUNDO;
        if ( flags.testFlag( ForceDelete ) )
            operationFlags |= FOF_NOCONFIRMATION;

        fileOperation->SetOperationFlags( operationFlags );

        for ( int i = 0; i < d->m_sourceItems.count(); i++ ) {
            IShellItem* item;
            hr = SHCreateItemWithParent( NULL, d->m_sourceFolder->d->m_folder, d->m_sourceItems.at( i ).d->m_pidl, IID_PPV_ARGS( &item ) );

            if ( SUCCEEDED( hr ) ) {
                hr = fileOperation->DeleteItem( item, NULL );

                item->Release();
            }

            if ( FAILED( hr ) )
                break;
        }

        if ( SUCCEEDED( hr ) ) {
            hr = fileOperation->PerformOperations();

            if ( SUCCEEDED( hr ) )
                result = true;
        }

        fileOperation->Release();
    }

    return result;
}

class ContextMenuGlobal
{
public:
    ContextMenuGlobal();
    ~ContextMenuGlobal();

public:
    IContextMenu2* m_menu;
    WNDPROC m_oldProc;
};

ContextMenuGlobal::ContextMenuGlobal() :
    m_menu( NULL ),
    m_oldProc( NULL )
{
}

ContextMenuGlobal::~ContextMenuGlobal()
{
}

Q_GLOBAL_STATIC( ContextMenuGlobal, contextMenuGlobal )

static LRESULT CALLBACK ContextMenuProc( HWND window, UINT message, WPARAM wparam, LPARAM lparam )
{
    ContextMenuGlobal* g = contextMenuGlobal();

    switch (message ) { 
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
            if ( !wparam ) {
                g->m_menu->HandleMenuMsg( message, wparam, lparam );
                return 0;
            }
            break;

        case WM_INITMENUPOPUP:
            g->m_menu->HandleMenuMsg( message, wparam, lparam );
            return TRUE;

        default:
            break;
    }

    return CallWindowProc( g->m_oldProc, window, message, wparam, lparam );
}

ShellSelection::MenuCommand ShellSelection::showContextMenu( const QPoint& pos, Flags flags )
{
    MenuCommand result = NoCommand;

    ContextMenuGlobal* g = contextMenuGlobal();

    QVector<LPCITEMIDLIST> pidls( d->m_sourceItems.count() );
    for ( int i = 0; i < d->m_sourceItems.count(); i++ )
        pidls[ i ] = d->m_sourceItems.at( i ).d->m_pidl;

    IContextMenu* contextMenu;
    HRESULT hr = d->m_sourceFolder->d->m_folder->GetUIObjectOf( parent()->effectiveWinId(), pidls.count(), pidls.data(), IID_IContextMenu, NULL, (void**)&contextMenu );

    if ( SUCCEEDED( hr ) ) {
        HMENU menu = CreatePopupMenu();

        UINT menuFlags = CMF_EXPLORE;

        if ( QApplication::keyboardModifiers().testFlag( Qt::ShiftModifier ) )
            menuFlags |= CMF_EXTENDEDVERBS;

        if ( flags.testFlag( CanRename ) )
            menuFlags |= CMF_CANRENAME;

        hr = contextMenu->QueryContextMenu( menu, 0, 1000, 0x7ffff, menuFlags );

        if ( SUCCEEDED( hr ) ) {
            hr = contextMenu->QueryInterface( IID_PPV_ARGS( &g->m_menu ) );
            if ( SUCCEEDED( hr ) )
                g->m_oldProc = (WNDPROC)SetWindowLongPtr( parent()->effectiveWinId(), GWLP_WNDPROC, (LONG_PTR)ContextMenuProc );

            int command = TrackPopupMenu( menu, TPM_RETURNCMD, pos.x(), pos.y(), 0, parent()->effectiveWinId(), NULL );

            if ( SUCCEEDED( hr ) ) {
                SetWindowLongPtr( parent()->effectiveWinId(), GWLP_WNDPROC, (LONG_PTR)g->m_oldProc );

                g->m_menu->Release();
                g->m_menu = NULL;
            }

            if ( command > 0 ) {
                result = InternalCommand;

                char buffer[ 256 ];
                hr = contextMenu->GetCommandString( command - 1000, GCS_VERBA, NULL, buffer, 256 );

                if ( SUCCEEDED( hr ) ) {
                    if ( flags.testFlag( CanOpen ) && !_stricmp( buffer, "open" ) )
                        result = Open;
                    else if ( flags.testFlag( CanRename ) && !_stricmp( buffer, "rename" ) )
                        result = Rename;
                }

                if ( result == InternalCommand ) {
                    CMINVOKECOMMANDINFO info = { 0 };
                    info.cbSize = sizeof( info );
                    info.hwnd = parent()->effectiveWinId();
                    info.lpVerb = MAKEINTRESOURCEA( command - 1000 );
                    info.nShow = SW_SHOWNORMAL;

                    hr = contextMenu->InvokeCommand( &info );

                    if ( FAILED( hr ) )
                        result = NoCommand;
                }
            }
        }

        DestroyMenu( menu );

        contextMenu->Release();
    }

    return result;
}

bool ShellSelection::invokeCommand( const char* verb )
{
    bool result = false;

    QVector<LPCITEMIDLIST> pidls( d->m_sourceItems.count() );
    for ( int i = 0; i < d->m_sourceItems.count(); i++ )
        pidls[ i ] = d->m_sourceItems.at( i ).d->m_pidl;

    IContextMenu* contextMenu;
    HRESULT hr = d->m_sourceFolder->d->m_folder->GetUIObjectOf( parent()->effectiveWinId(), pidls.count(), pidls.data(), IID_IContextMenu, NULL, (void**)&contextMenu );

    if ( SUCCEEDED( hr ) ) {
        CMINVOKECOMMANDINFO info = { 0 };
        info.cbSize = sizeof( info );
        info.hwnd = parent()->effectiveWinId();
        info.lpVerb = verb;
        info.nShow = SW_SHOWNORMAL;

        hr = contextMenu->InvokeCommand( &info );

        if ( SUCCEEDED( hr ) )
            result = true;

        contextMenu->Release();
    }

    return result;
}
