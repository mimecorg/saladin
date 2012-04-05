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

#ifndef SHELLITEM_P_H
#define SHELLITEM_P_H

#include "shellitem.h"

#include <shlobj.h>

class ShellItemPrivate : public QSharedData
{
public:
    ShellItemPrivate();
    ShellItemPrivate( const ShellItemPrivate& other );
    ~ShellItemPrivate();

public:
    LPITEMIDLIST m_pidl;

    QString m_name;
    qint64 m_size;
    QDateTime m_modified;
    QPixmap m_icon;

    ShellItem::Attributes m_attributes;
    ShellItem::State m_state;
};

#endif
