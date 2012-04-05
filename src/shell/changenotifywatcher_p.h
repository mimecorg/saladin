/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2012 Michał Męciński
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

#ifndef CHANGENOTIFYWATCHER_P_H
#define CHANGENOTIFYWATCHER_P_H

#include <QObject>

#include <shlobj.h>

class ChangeNotifyWatcher : public QObject
{
    Q_OBJECT
public:
    ChangeNotifyWatcher( LPITEMIDLIST pidl, int eventTypes, QObject* parent );
    ~ChangeNotifyWatcher();

public:
    bool isValid() const;

signals:
    void changeNotify( int eventType, void* arg1, void* arg2 );

private:
    friend LRESULT CALLBACK ChangeNotifyWatcherProc( HWND window, UINT message, WPARAM wparam, LPARAM lparam );

private:
    HWND m_window;

    ULONG m_registerId;
};

#endif
