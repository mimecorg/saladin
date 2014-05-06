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

#include "shellpidl.h"
#include "shellpidl_p.h"

ShellPidlPrivate::ShellPidlPrivate()
{
}

ShellPidlPrivate::ShellPidlPrivate( const ShellPidlPrivate& other ) : QSharedData( other ),
    m_data( other.m_data ),
    m_path( other.m_path )
{
}

ShellPidlPrivate::~ShellPidlPrivate()
{
}

ShellPidl::ShellPidl() :
    d( new ShellPidlPrivate() )
{
}

ShellPidl::~ShellPidl()
{
}

ShellPidl::ShellPidl( const ShellPidl& other ) :
    d( other.d )
{
}

ShellPidl& ShellPidl::operator =( const ShellPidl& other )
{
    d = other.d;
    return *this;
}

bool ShellPidl::isValid() const
{
    return !d->m_data.isEmpty();
}

QString ShellPidl::path() const
{
    return d->m_path;
}

bool operator ==( const ShellPidl& lhs, const ShellPidl& rhs )
{
    return ILIsEqual( lhs.d->pidl(), rhs.d->pidl() );
}

bool operator !=( const ShellPidl& lhs, const ShellPidl& rhs )
{
    return !( lhs == rhs );
}

QDataStream& operator <<( QDataStream& stream, const ShellPidl& pidl )
{
    return stream << pidl.d->m_data << pidl.d->m_path;
}

QDataStream& operator >>( QDataStream& stream, ShellPidl& pidl )
{
    return stream >> pidl.d->m_data >> pidl.d->m_path;
}

void ShellPidl::registerMetaType()
{
    qRegisterMetaTypeStreamOperators<ShellPidl>( "ShellPidl" );
}
