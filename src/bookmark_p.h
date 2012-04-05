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

#ifndef BOOKMARK_P_H
#define BOOKMARK_P_H

#include "bookmark.h"

#include "shell/shellpidl.h"

class BookmarkPrivate : public QSharedData
{
public:
    enum BookmarkType
    {
        NullBookmark,
        PidlBookmark,
        FtpBookmark
    };

public:
    BookmarkPrivate();
    BookmarkPrivate( const BookmarkPrivate& other );
    ~BookmarkPrivate();

public:
    QString m_name;

    BookmarkType m_type;

    ShellPidl m_pidl;

    QString m_path;
    QString m_user;
    QString m_password;
};

#endif
