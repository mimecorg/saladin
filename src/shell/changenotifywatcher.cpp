/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2013 Michał Męciński
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

#include "changenotifywatcher_p.h"

static const wchar_t ChangeNotifyWatcher_Window[] = L"ChangeNotifyWatcher_Window";
static const wchar_t ChangeNotifyWatcher_Message[] = L"ChangeNotifyWatcher_Message";

class ChangeNotifyWatcherGlobal
{
public:
    ChangeNotifyWatcherGlobal();
    ~ChangeNotifyWatcherGlobal();

public:
    UINT m_message;

    QMap<HWND, ChangeNotifyWatcher*> m_watchers;
};

ChangeNotifyWatcherGlobal::ChangeNotifyWatcherGlobal()
{
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = ChangeNotifyWatcherProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = qWinAppInst();
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = ChangeNotifyWatcher_Window;

    RegisterClass( &wc );

    m_message = RegisterWindowMessage( ChangeNotifyWatcher_Message );
}

ChangeNotifyWatcherGlobal::~ChangeNotifyWatcherGlobal()
{
}

Q_GLOBAL_STATIC( ChangeNotifyWatcherGlobal, changeNotifyWatcherGlobal )

ChangeNotifyWatcher::ChangeNotifyWatcher( LPITEMIDLIST pidl, int eventTypes, QObject* parent ) : QObject( parent ),
    m_window( NULL ),
    m_registerId( 0 )
{
    ChangeNotifyWatcherGlobal* g = changeNotifyWatcherGlobal();

    m_window = CreateWindow( ChangeNotifyWatcher_Window, ChangeNotifyWatcher_Window, 0, 0, 0, 0, 0, NULL, NULL, qWinAppInst(), NULL );

    if ( m_window ) {
        SHChangeNotifyEntry entry;
        entry.pidl = pidl;
        entry.fRecursive = false;

        int sources = SHCNRF_InterruptLevel | SHCNRF_ShellLevel | SHCNRF_NewDelivery;

        m_registerId = SHChangeNotifyRegister( m_window, sources, eventTypes, g->m_message, 1, &entry );

        if ( m_registerId )
            g->m_watchers.insert( m_window, this );
    }
}

ChangeNotifyWatcher::~ChangeNotifyWatcher()
{
    ChangeNotifyWatcherGlobal* g = changeNotifyWatcherGlobal();

    if ( m_registerId ) {
        SHChangeNotifyDeregister( m_registerId );

        g->m_watchers.remove( m_window );
    }

    if ( m_window )
        DestroyWindow( m_window );
}

bool ChangeNotifyWatcher::isValid() const
{
    return m_registerId != 0;
}

static LRESULT CALLBACK ChangeNotifyWatcherProc( HWND window, UINT message, WPARAM wparam, LPARAM lparam )
{
    ChangeNotifyWatcherGlobal* g = changeNotifyWatcherGlobal();

    if ( message == g->m_message ) {
        ChangeNotifyWatcher* watcher = g->m_watchers.value( window );

        if ( watcher ) {
            long eventType;
            LPITEMIDLIST* args;
            HANDLE lock = SHChangeNotification_Lock( (HANDLE)wparam, (DWORD)lparam, &args, &eventType );

            if ( lock ) {
                emit watcher->changeNotify( eventType, args[ 0 ], args[ 1 ] );

                SHChangeNotification_Unlock( lock );
            }
        }

        return 0;
    }

    return DefWindowProc( window, message, wparam, lparam );
}
