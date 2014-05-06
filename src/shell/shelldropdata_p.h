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

#ifndef SHELLDROPDATA_P_H
#define SHELLDROPDATA_P_H

#include "shelldropdata.h"
#include "shellitem.h"
#include "shelldrive.h"

class ShellDropDataPrivate
{
public:
    enum Target
    {
        NoTarget,
        FolderTarget,
        ParentTarget,
        ItemTarget,
        DriveTarget
    };

public:
    ShellDropDataPrivate();
    ~ShellDropDataPrivate();

public:
    bool dragOver( QDropEvent* e );

    void dragLeave();

public:
    ShellDropData* q;

    IDataObject* m_dataObject;

    Target m_target;

    ShellFolder* m_folder;
    ShellComputer* m_computer;

    ShellItem m_item;
    ShellDrive m_drive;

    bool m_ignoreItem;

    IDropTarget* m_dropTarget;
    LPITEMIDLIST m_targetPidl;

    POINTL m_point;
    DWORD m_keyState;
    DWORD m_possibleEffect;

    bool m_dragEntered;
    Qt::DropAction m_dropAction;
};

#endif
