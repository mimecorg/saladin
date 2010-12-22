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

#ifndef SHELLDROPDATA_P_H
#define SHELLDROPDATA_P_H

#include "shelldropdata.h"

class ShellDropDataPrivate
{
public:
    ShellDropDataPrivate();
    ~ShellDropDataPrivate();

public:
    bool dragDropHelper( QDropEvent* e, ShellFolder* folder, bool doDrop );
    bool dragDropHelper( QDropEvent* e, ShellFolder* folder, const ShellItem& item, bool doDrop );
    bool dragDropHelper( QDropEvent* e, IDropTarget* dropTarget, LPITEMIDLIST targetPidl, bool doDrop );

public:
    ShellDropData* q;

    IDataObject* m_dataObject;

    Qt::DropActions m_possibleActions;
    Qt::DropAction m_dropAction;
};

#endif
