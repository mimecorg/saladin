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

#ifndef SHELLPIDL_H
#define SHELLPIDL_H

#include <QSharedData>

class ShellPidlPrivate;

class ShellPidl
{
public:
    ShellPidl();
    ~ShellPidl();

    ShellPidl( const ShellPidl& other );
    ShellPidl& operator =( const ShellPidl& other );

public:
    bool isValid() const;

    QString path() const;

    friend bool operator ==( const ShellPidl& lhs, const ShellPidl& rhs );
    friend bool operator !=( const ShellPidl& lhs, const ShellPidl& rhs );

private:
    QSharedDataPointer<ShellPidlPrivate> d;

    friend class ShellFolder;
};

#endif
