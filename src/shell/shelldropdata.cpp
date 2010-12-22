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
#include "shellitem_p.h"

#include <private/qdnd_p.h>

ShellDropDataPrivate::ShellDropDataPrivate() :
    q( NULL ),
    m_dataObject( NULL ),
    m_dropAction( Qt::IgnoreAction )
{
}

ShellDropDataPrivate::~ShellDropDataPrivate()
{
    if ( m_dataObject ) {
        m_dataObject->Release();
        m_dataObject = NULL;
    }
}

ShellDropData::ShellDropData( QDropEvent* e, QWidget* parent ) : QObject( parent ),
    d( new ShellDropDataPrivate() )
{
    d->q = this;

    if ( e->mimeData()->inherits( "QDropData" ) ) {
        const QDropData* dropData = static_cast<const QDropData*>( e->mimeData() );
        d->m_dataObject = dropData->currentDataObject;
        d->m_dataObject->AddRef();

        d->m_possibleActions = e->possibleActions();
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

bool ShellDropData::dragMove( QDragMoveEvent* e, ShellFolder* folder )
{
    return d->dragDropHelper( e, folder, false );
}

bool ShellDropData::dragMove( QDragMoveEvent* e, ShellFolder* folder, const ShellItem& item )
{
    return d->dragDropHelper( e, folder, item, false );
}

bool ShellDropData::drop( QDropEvent* e, ShellFolder* folder )
{
    return d->dragDropHelper( e, folder, true );
}

bool ShellDropData::drop( QDropEvent* e, ShellFolder* folder, const ShellItem& item )
{
    return d->dragDropHelper( e, folder, item, true );
}

bool ShellDropDataPrivate::dragDropHelper( QDropEvent* e, ShellFolder* folder, bool doDrop )
{
    bool result = false;

    IDropTarget* dropTarget;
    HRESULT hr = folder->d->m_folder->CreateViewObject( q->parent()->effectiveWinId(), IID_PPV_ARGS( &dropTarget ) );

    if ( SUCCEEDED( hr ) ) {
        result = dragDropHelper( e, dropTarget, folder->d->m_pidl, doDrop );

        dropTarget->Release();
    }

    return result;
}

bool ShellDropDataPrivate::dragDropHelper( QDropEvent* e, ShellFolder* folder, const ShellItem& item, bool doDrop )
{
    bool result = false;

    IDropTarget* dropTarget;
    HRESULT hr = folder->d->m_folder->GetUIObjectOf( q->parent()->effectiveWinId(), 1, (LPCITEMIDLIST*)&item.d->m_pidl, IID_IDropTarget, NULL, (void**)&dropTarget );

    if ( SUCCEEDED( hr ) ) {
        LPITEMIDLIST absolutePidl = ILCombine( folder->d->m_pidl, item.d->m_pidl );

        result = dragDropHelper( e, dropTarget, absolutePidl, doDrop );

        CoTaskMemFree( absolutePidl );
        dropTarget->Release();
    }

    return result;
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

bool ShellDropDataPrivate::dragDropHelper( QDropEvent* e, IDropTarget* dropTarget, LPITEMIDLIST targetPidl, bool doDrop )
{
    bool result = false;
    m_dropAction = Qt::IgnoreAction;

    QPoint position = QCursor::pos();
    POINTL point = { position.x(), position.y() };

    DWORD keyState = makeKeyState( e->mouseButtons(), e->keyboardModifiers() );

    DWORD possibleEffect = dropActionsToEffect( m_possibleActions );

    DWORD dragEffect = possibleEffect;
    HRESULT hr = dropTarget->DragEnter( m_dataObject, keyState, point, &dragEffect );

    if ( SUCCEEDED( hr ) && ( dragEffect & possibleEffect ) != 0 ) {
        if ( doDrop ) {
            DWORD dropEffect = possibleEffect;
            hr = dropTarget->Drop( m_dataObject, keyState, point, &dropEffect );

            if ( SUCCEEDED( hr ) ) {
                SHChangeNotify( SHCNE_UPDATEDIR, SHCNF_IDLIST, targetPidl, NULL );

                m_dropAction = effectToDropAction( dropEffect );
                result = true;
            }
        } else {
            dropTarget->DragLeave();

            m_dropAction = effectToDropAction( dragEffect );
            result = true;
        }
    }

    return result;
}
