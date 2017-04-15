/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2017 Michał Męciński
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

#ifndef SHELLFOLDER_P_H
#define SHELLFOLDER_P_H

#include "shellbase_p.h"

#include <shlobj.h>

class ShellFolder;

class ShellFolderPrivate : public ShellBasePrivate
{
public:
    ShellFolderPrivate();
    ~ShellFolderPrivate();

public:
    void readProperties();

    ShellItem makeItem( LPITEMIDLIST pidl );
    ShellItem makeNotifyItem( LPITEMIDLIST pidl );
    ShellItem makeRealNotifyItem( LPITEMIDLIST pidl );

    void readItemProperties( ShellItem& item );

    ShellFolder* createParentFolder( ShellItem& item );

    qint64 calculateSize( IShellFolder* parentFolder, LPITEMIDLIST pidl );

public:
    ShellFolder* q;

    QString m_path;

    QString m_user;
    QString m_password;

    bool m_hasParent;
};

#endif
