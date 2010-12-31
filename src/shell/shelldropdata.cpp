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

#include "shelldropdata.h"
#include "shelldropdata_p.h"
#include "shellfolder.h"
#include "shellfolder_p.h"
#include "shellcomputer.h"
#include "shellcomputer_p.h"
#include "shellitem_p.h"
#include "shelldrive_p.h"

#include <private/qdnd_p.h>

ShellDropDataPrivate::ShellDropDataPrivate() :
    q( NULL ),
    m_dataObject( NULL ),
    m_target( NoTarget ),
    m_folder( NULL ),
    m_computer( NULL ),
    m_ignoreItem( false ),
    m_dropTarget( NULL ),
    m_targetPidl( NULL ),
    m_dragEntered( false ),
    m_dropAction( Qt::IgnoreAction )
{
}

ShellDropDataPrivate::~ShellDropDataPrivate()
{
    if ( m_dataObject ) {
        m_dataObject->Release();
        m_dataObject = NULL;
    }

    dragLeave();
}

ShellDropData::ShellDropData( QDropEvent* e, ShellFolder* folder, QWidget* parent ) : QObject( parent ),
    d( new ShellDropDataPrivate() )
{
    d->q = this;

    if ( e->mimeData()->inherits( "QDropData" ) ) {
        const QDropData* dropData = static_cast<const QDropData*>( e->mimeData() );

        d->m_dataObject = dropData->currentDataObject;
        d->m_dataObject->AddRef();

        d->m_folder = folder;
    }
}

ShellDropData::ShellDropData( QDropEvent* e, ShellComputer* computer, QWidget* parent ) : QObject( parent ),
    d( new ShellDropDataPrivate() )
{
    d->q = this;

    if ( e->mimeData()->inherits( "QDropData" ) ) {
        const QDropData* dropData = static_cast<const QDropData*>( e->mimeData() );

        d->m_dataObject = dropData->currentDataObject;
        d->m_dataObject->AddRef();

        d->m_computer = computer;
    }
}

ShellDropData::~ShellDropData()
{
    delete d;
}

bool ShellDropData::isValid() const
{
    return d->m_dataObject != NULL;
}

Qt::DropAction ShellDropData::dropAction() const
{
    return d->m_dropAction;
}

bool ShellDropData::dragToFolder( QDropEvent* e )
{
    if ( d->m_target != ShellDropDataPrivate::FolderTarget ) {
        d->dragLeave();

        HRESULT hr = d->m_folder->d->m_folder->CreateViewObject( parent()->effectiveWinId(), IID_PPV_ARGS( &d->m_dropTarget ) );

        if ( SUCCEEDED( hr ) ) {
            d->m_targetPidl = ILClone( d->m_folder->d->m_pidl );

            d->m_target = ShellDropDataPrivate::FolderTarget;
        }
    }

    return d->dragOver( e );
}

bool ShellDropData::dragToParent( QDropEvent* e )
{
    if ( d->m_target != ShellDropDataPrivate::ParentTarget ) {
        d->dragLeave();

        IShellFolder* folder;
        LPCITEMIDLIST pidlLast;
        HRESULT hr = SHBindToParent( d->m_folder->d->m_pidl, IID_PPV_ARGS( &folder ), &pidlLast );

        if ( SUCCEEDED( hr ) ) {
            hr = folder->CreateViewObject( parent()->effectiveWinId(), IID_PPV_ARGS( &d->m_dropTarget ) );

            if ( SUCCEEDED( hr ) ) {
                SHGetIDListFromObject( folder, &d->m_targetPidl );

                d->m_target = ShellDropDataPrivate::ParentTarget;
            }
        }

        folder->Release();
    }

    return d->dragOver( e );
}

bool ShellDropData::dragToItem( QDropEvent* e, const ShellItem& item )
{
    if ( d->m_ignoreItem && item == d->m_item )
        return false;

    if ( d->m_target != ShellDropDataPrivate::ItemTarget || item != d->m_item ) {
        d->dragLeave();

        d->m_item = item;

        HRESULT hr = d->m_folder->d->m_folder->GetUIObjectOf( parent()->effectiveWinId(), 1, (LPCITEMIDLIST*)&item.d->m_pidl, IID_IDropTarget, NULL, (void**)&d->m_dropTarget );

        if ( SUCCEEDED( hr ) ) {
            d->m_targetPidl = ILCombine( d->m_folder->d->m_pidl, item.d->m_pidl );

            d->m_target = ShellDropDataPrivate::ItemTarget;
        }
    }

    bool result = d->dragOver( e );

    d->m_ignoreItem = !result;

    return result;
}

bool ShellDropData::dragToDrive( QDropEvent* e, const ShellDrive& drive )
{
    if ( d->m_target != ShellDropDataPrivate::DriveTarget || drive != d->m_drive ) {
        d->dragLeave();

        d->m_drive = drive;

        HRESULT hr = d->m_computer->d->m_folder->GetUIObjectOf( parent()->effectiveWinId(), 1, (LPCITEMIDLIST*)&drive.d->m_pidl, IID_IDropTarget, NULL, (void**)&d->m_dropTarget );

        if ( SUCCEEDED( hr ) ) {
            d->m_targetPidl = ILCombine( d->m_computer->d->m_pidl, drive.d->m_pidl );

            d->m_target = ShellDropDataPrivate::DriveTarget;
        }
    }

    return d->dragOver( e );
}

static DWORD dropActionsToEffect( Qt::DropActions actions )
{
    DWORD effect = DROPEFFECT_NONE;
    if ( actions & Qt::LinkAction )
        effect |= DROPEFFECT_LINK;
    if ( actions & Qt::CopyAction )
        effect |= DROPEFFECT_COPY;
    if ( actions & Qt::MoveAction )
        effect |= DROPEFFECT_MOVE;
    return effect;
}

static Qt::DropAction effectToDropAction( DWORD effect )
{
    if ( effect & DROPEFFECT_LINK )
        return Qt::LinkAction;
    if ( effect & DROPEFFECT_COPY )
        return Qt::CopyAction;
    if ( effect & DROPEFFECT_MOVE )
        return Qt::MoveAction;
    return Qt::IgnoreAction;
}

static DWORD makeKeyState( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers )
{
    DWORD state = 0;
    if ( buttons.testFlag( Qt::LeftButton ) )
        state |= MK_LBUTTON;
    if ( buttons.testFlag( Qt::RightButton ) )
        state |= MK_RBUTTON;
    if ( buttons.testFlag( Qt::MiddleButton ) )
        state |= MK_MBUTTON;
    if ( buttons.testFlag( Qt::XButton1 ) )
        state |= MK_XBUTTON1;
    if ( buttons.testFlag( Qt::XButton2 ) )
        state |= MK_XBUTTON2;
    if ( modifiers.testFlag( Qt::ShiftModifier ) )
        state |= MK_SHIFT;
    if ( modifiers.testFlag( Qt::ControlModifier ) )
        state |= MK_CONTROL;
    if ( modifiers.testFlag( Qt::AltModifier ) )
        state |= MK_ALT;
    return state;
}

bool ShellDropDataPrivate::dragOver( QDropEvent* e )
{
    if ( m_target == ShellDropDataPrivate::NoTarget )
        return false;

    bool result = false;

    QPoint position = QCursor::pos();
    m_point.x = position.x();
    m_point.y = position.y();

    m_keyState = makeKeyState( e->mouseButtons(), e->keyboardModifiers() );

    m_possibleEffect = dropActionsToEffect( e->possibleActions() );

    DWORD dragEffect = m_possibleEffect;
    HRESULT hr;
    if ( !m_dragEntered )
        hr = m_dropTarget->DragEnter( m_dataObject, m_keyState, m_point, &dragEffect );
    else
        hr = m_dropTarget->DragOver( m_keyState, m_point, &dragEffect );

    if ( SUCCEEDED( hr ) ) {
        m_dragEntered = true;
        m_dropAction = effectToDropAction( dragEffect & m_possibleEffect );

        if ( m_dropAction != Qt::IgnoreAction )
            result = true;
    }

    return result;
}

bool ShellDropData::drop()
{
    if ( !d->m_dragEntered || d->m_dropAction == Qt::IgnoreAction )
        return false;

    bool result = false;

    DWORD dropEffect = d->m_possibleEffect;
    HRESULT hr = d->m_dropTarget->Drop( d->m_dataObject, d->m_keyState, d->m_point, &dropEffect );

    if ( SUCCEEDED( hr ) ) {
        if ( d->m_targetPidl != NULL )
            SHChangeNotify( SHCNE_UPDATEDIR, SHCNF_IDLIST, d->m_targetPidl, NULL );

        d->m_dropAction = effectToDropAction( dropEffect );
        result = true;
    }

    return result;
}

void ShellDropDataPrivate::dragLeave()
{
    if ( m_dropTarget ) {
        if ( m_dragEntered )
            m_dropTarget->DragLeave();

        m_dropTarget->Release();
        m_dropTarget = NULL;
    }

    if ( m_targetPidl ) {
        CoTaskMemFree( m_targetPidl );
        m_targetPidl = NULL;
    }

    m_target = NoTarget;

    m_dragEntered = false;
    m_dropAction = Qt::IgnoreAction;
}
