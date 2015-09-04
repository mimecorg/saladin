/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2014 Michał Męciński
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

WindowDropTarget* ShellDropDataPrivate::m_windowDropTarget = NULL;

IDataObject* ShellDropDataPrivate::m_dropDataObject = NULL;
QMimeData* ShellDropDataPrivate::m_dropMimeData = NULL;

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

    if ( e->mimeData() == ShellDropDataPrivate::m_dropMimeData ) {
        d->m_dataObject = ShellDropDataPrivate::m_dropDataObject;
        d->m_dataObject->AddRef();

        d->m_folder = folder;
    }
}

ShellDropData::ShellDropData( QDropEvent* e, ShellComputer* computer, QWidget* parent ) : QObject( parent ),
    d( new ShellDropDataPrivate() )
{
    d->q = this;

    if ( e->mimeData() == ShellDropDataPrivate::m_dropMimeData ) {
        d->m_dataObject = ShellDropDataPrivate::m_dropDataObject;
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

        HRESULT hr = d->m_folder->d->m_folder->CreateViewObject( (HWND)parent()->effectiveWinId(), IID_PPV_ARGS( &d->m_dropTarget ) );

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
            hr = folder->CreateViewObject( (HWND)parent()->effectiveWinId(), IID_PPV_ARGS( &d->m_dropTarget ) );

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

        HRESULT hr = d->m_folder->d->m_folder->GetUIObjectOf( (HWND)parent()->effectiveWinId(), 1, (LPCITEMIDLIST*)&item.d->m_pidl, IID_IDropTarget, NULL, (void**)&d->m_dropTarget );

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

        HRESULT hr = d->m_computer->d->m_folder->GetUIObjectOf( (HWND)parent()->effectiveWinId(), 1, (LPCITEMIDLIST*)&drive.d->m_pidl, IID_IDropTarget, NULL, (void**)&d->m_dropTarget );

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

class WindowDropTarget : public IDropTarget
{
public:
    WindowDropTarget( QWindow* window );
    ~WindowDropTarget();

public: // IUnknown methods
    STDMETHOD( QueryInterface )( REFIID iid, void** ppv );
    STDMETHOD_( ULONG, AddRef )( );
    STDMETHOD_( ULONG, Release )( );

    // IDropTarget methods
    STDMETHOD( DragEnter )( IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect );
    STDMETHOD( DragOver )( DWORD keyState, POINTL pt, DWORD* effect );
    STDMETHOD( DragLeave )( );
    STDMETHOD( Drop )( IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect );

private:
    void dragHelper( DWORD keyState, POINTL pt, DWORD* effect, bool enter );

    void clear();

private:
    QWindow* m_window;

    ULONG m_refs;

    DWORD m_lastKeyState;
    POINTL m_lastPoint;
    DWORD m_lastEffect;

    Qt::DropAction m_lastAcceptedAction;
};

WindowDropTarget::WindowDropTarget( QWindow* window ) :
    m_window( window ),
    m_refs( 1 ),
    m_lastKeyState( 0 ),
    m_lastEffect( DROPEFFECT_NONE ),
    m_lastAcceptedAction( Qt::IgnoreAction )
{
    m_lastPoint.x = 0;
    m_lastPoint.y = 0;
}

WindowDropTarget::~WindowDropTarget()
{
    clear();
}

STDMETHODIMP WindowDropTarget::QueryInterface( REFIID iid, void** ppv )
{
    if ( iid == IID_IUnknown || iid == IID_IDropTarget ) {
        *ppv = this;
        ++m_refs;
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP_( ULONG ) WindowDropTarget::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_( ULONG ) WindowDropTarget::Release()
{
    if ( --m_refs == 0 ) {
        delete this;
        return 0;
    }
    return m_refs;
}

static Qt::DropActions effectToDropActions( DWORD effect )
{
    Qt::DropActions actions = 0;
    if ( effect & DROPEFFECT_LINK )
        actions |= Qt::LinkAction;
    if ( effect & DROPEFFECT_COPY )
        actions |= Qt::CopyAction;
    if ( effect & DROPEFFECT_MOVE )
        actions |= Qt::MoveAction;
    return actions;
}

static DWORD dropActionToEffect( Qt::DropAction action )
{
    if ( action == Qt::LinkAction )
        return DROPEFFECT_LINK;
    if ( action == Qt::CopyAction )
        return DROPEFFECT_COPY;
    if ( action == Qt::MoveAction )
        return DROPEFFECT_MOVE;
    return DROPEFFECT_NONE;
}

static Qt::MouseButtons keyStateToMouseButtons( DWORD keyState )
{
    Qt::MouseButtons buttons = 0;
    if ( keyState & MK_LBUTTON )
        buttons |= Qt::LeftButton;
    if ( keyState & MK_RBUTTON )
        buttons |= Qt::RightButton;
    if ( keyState & MK_MBUTTON )
        buttons |= Qt::MiddleButton;
    if ( keyState & MK_XBUTTON1 )
        buttons |= Qt::XButton1;
    if ( keyState & MK_XBUTTON2 )
        buttons |= Qt::XButton2;
    return buttons;
}

static Qt::KeyboardModifiers keyStateToKeyboardModifiers( DWORD keyState )
{
    Qt::KeyboardModifiers modifiers = 0;
    if ( keyState & MK_SHIFT )
        modifiers |= Qt::ShiftModifier;
    if ( keyState & MK_CONTROL )
        modifiers |= Qt::ControlModifier;
    if ( keyState & MK_ALT )
        modifiers |= Qt::AltModifier;
    return modifiers;
}

STDMETHODIMP WindowDropTarget::DragEnter( IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect )
{
    ShellDropDataPrivate::m_dropDataObject = dataObject;

    ShellDropDataPrivate::m_dropMimeData = new QMimeData();

    FORMATETC formatetc;
    formatetc.cfFormat = CF_TEXT;
    formatetc.dwAspect = DVASPECT_CONTENT;
    formatetc.lindex = -1;
    formatetc.ptd = NULL;
    formatetc.tymed = TYMED_HGLOBAL;

    if ( SUCCEEDED( dataObject->QueryGetData( &formatetc ) ) ) {
        STGMEDIUM stg;

        if ( SUCCEEDED( dataObject->GetData( &formatetc, &stg ) ) ) {
            DWORD* data = (DWORD*)GlobalLock( stg.hGlobal );
            QString text = QString::fromLocal8Bit( (const char*)data, GlobalSize( stg.hGlobal ) );
            GlobalUnlock( stg.hGlobal );
            ReleaseStgMedium( &stg );

            ShellDropDataPrivate::m_dropMimeData->setText( text );
        }
    }

    dragHelper( keyState, pt, effect, true );

    return S_OK;
}

STDMETHODIMP WindowDropTarget::DragOver( DWORD keyState, POINTL pt, DWORD* effect )
{
    if ( keyState == m_lastKeyState && pt.x == m_lastPoint.x && pt.y == m_lastPoint.y ) {
        *effect = m_lastEffect;
        return S_OK;
    }

    dragHelper( keyState, pt, effect, false );

    return S_OK;
}

void WindowDropTarget::dragHelper( DWORD keyState, POINTL pt, DWORD* effect, bool enter )
{
    m_lastKeyState = keyState;
    m_lastPoint = pt;

    QPoint point = m_window->mapFromGlobal( QPoint( pt.x, pt.y ) );
    Qt::DropActions actions = effectToDropActions( *effect );
    Qt::MouseButtons buttons = keyStateToMouseButtons( keyState );
    Qt::KeyboardModifiers modifiers = keyStateToKeyboardModifiers( keyState );

    QDragMoveEvent me( point, actions, ShellDropDataPrivate::m_dropMimeData, buttons, modifiers );

    if ( enter ) {
        QDragEnterEvent e( point, actions, ShellDropDataPrivate::m_dropMimeData, buttons, modifiers );
        QApplication::sendEvent( m_window, &e );
        if ( e.isAccepted() && e.dropAction() != Qt::IgnoreAction )
            m_lastAcceptedAction = e.dropAction();
    }

    if ( m_lastAcceptedAction != Qt::IgnoreAction && ( m_lastAcceptedAction & actions ) ) {
        me.setDropAction( m_lastAcceptedAction );
        me.accept();
    }

    QApplication::sendEvent( m_window, &me );

    if ( me.isAccepted() ) {
        m_lastAcceptedAction = me.dropAction();
        *effect = dropActionToEffect( m_lastAcceptedAction );
    } else {
        m_lastAcceptedAction = Qt::IgnoreAction;
        *effect = DROPEFFECT_NONE;
    }

    m_lastEffect = *effect;
}

STDMETHODIMP WindowDropTarget::DragLeave()
{
    QDragLeaveEvent e;
    QApplication::sendEvent( m_window, &e );

    clear();

    return S_OK;
}

STDMETHODIMP WindowDropTarget::Drop( IDataObject* /*dataObject*/, DWORD keyState, POINTL pt, DWORD* effect )
{
    if ( ( keyState & ( MK_LBUTTON | MK_MBUTTON | MK_RBUTTON ) ) == 0 )
        keyState |= m_lastKeyState & ( MK_LBUTTON | MK_MBUTTON | MK_RBUTTON );
    m_lastKeyState = keyState;
    m_lastPoint = pt;

    QDropEvent e( m_window->mapFromGlobal( QPoint( pt.x, pt.y ) ), effectToDropActions( *effect ), ShellDropDataPrivate::m_dropMimeData,
        keyStateToMouseButtons( keyState ), keyStateToKeyboardModifiers( keyState ) );
    QApplication::sendEvent( m_window, &e );

    if ( e.isAccepted() )
        *effect = dropActionToEffect( e.dropAction() );
    else
        *effect = DROPEFFECT_NONE;

    clear();

    return S_OK;
}

void WindowDropTarget::clear()
{
    ShellDropDataPrivate::m_dropDataObject = NULL;

    delete ShellDropDataPrivate::m_dropMimeData;
    ShellDropDataPrivate::m_dropMimeData = NULL;

    m_lastKeyState = 0;
    m_lastPoint.x = 0;
    m_lastPoint.y = 0;

    m_lastAcceptedAction = Qt::IgnoreAction;
}

void ShellDropData::registerDropTarget( QWindow* window )
{
    // unregister Qt built-in IDropTarget
    RevokeDragDrop( (HWND)window->winId() );

    ShellDropDataPrivate::m_windowDropTarget = new WindowDropTarget( window );

    // register custom IDropTarget
    RegisterDragDrop( (HWND)window->winId(), ShellDropDataPrivate::m_windowDropTarget );

    CoLockObjectExternal( ShellDropDataPrivate::m_windowDropTarget, true, true );
}

void ShellDropData::unregisterDropTarget( QWindow* window )
{
    if ( ShellDropDataPrivate::m_windowDropTarget ) {
        ShellDropDataPrivate::m_windowDropTarget->Release();

        CoLockObjectExternal( ShellDropDataPrivate::m_windowDropTarget, false, true );

        ShellDropDataPrivate::m_windowDropTarget = NULL;
    }

    RevokeDragDrop( (HWND)window->winId() );
}
