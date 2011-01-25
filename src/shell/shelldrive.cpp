/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011 Michał Męciński
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

#include "shelldrive.h"
#include "shelldrive_p.h"

ShellDrivePrivate::ShellDrivePrivate() :
    m_pidl( NULL )
{
}

ShellDrivePrivate::ShellDrivePrivate( const ShellDrivePrivate& other ) : QSharedData( other ),
    m_pidl( ILClone( other.m_pidl ) )
{
}

ShellDrivePrivate::~ShellDrivePrivate()
{
    if ( m_pidl ) {
        CoTaskMemFree( m_pidl );
        m_pidl = NULL;
    }
}

ShellDrive::ShellDrive() :
    d( new ShellDrivePrivate() )
{
}

ShellDrive::~ShellDrive()
{
}

ShellDrive::ShellDrive( const ShellDrive& other ) :
    d( other.d )
{
}

ShellDrive& ShellDrive::operator =( const ShellDrive& other )
{
    d = other.d;
    return *this;
}

bool ShellDrive::isValid() const
{
    return d->m_pidl != NULL;
}

char ShellDrive::letter() const
{
    return d->m_letter;
}

QString ShellDrive::name() const
{
    return d->m_name;
}

QPixmap ShellDrive::icon() const
{
    return d->m_icon;
}

bool ShellDrive::getFreeSpace( qint64* free, qint64* total ) const
{
    QString path = QLatin1Char( d->m_letter ) + QLatin1String( ":\\" );

    ULARGE_INTEGER freeBytes;
    ULARGE_INTEGER totalBytes;

    if ( GetDiskFreeSpaceEx( path.utf16(), &freeBytes, &totalBytes, NULL ) )
    {
        *free = freeBytes.QuadPart;
        *total = totalBytes.QuadPart;
        return true;
    }

    return false;
}

bool operator ==( const ShellDrive& lhs, const ShellDrive& rhs )
{
    return ILIsEqual( lhs.d->m_pidl, rhs.d->m_pidl );
}

bool operator !=( const ShellDrive& lhs, const ShellDrive& rhs )
{
    return !( lhs == rhs );
}
