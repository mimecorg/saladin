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

#ifndef SHELLSELECTION_P_H
#define SHELLSELECTION_P_H

#include <shlobj.h>

class ShellSelectionPrivate
{
public:
    ShellSelectionPrivate();
    ~ShellSelectionPrivate();

public:
    bool dragDropHelper( ShellFolder* targetFolder, ShellSelection::TransferType type, bool doDrop );

public:
    ShellSelection* q;

    ShellFolder* m_sourceFolder;
    QList<ShellItem> m_sourceItems;

    static ShellSelection* m_dragSelection;
    static IDataObject* m_dragObject;
};

#endif
