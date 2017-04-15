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

#include "shellitem.h"
#include "shellitem_p.h"

ShellItemPrivate::ShellItemPrivate() :
    m_pidl( NULL ),
    m_size( 0 ),
    m_attributes( 0 ),
    m_state( 0 )
{
}

ShellItemPrivate::ShellItemPrivate( const ShellItemPrivate& other ) : QSharedData( other ),
    m_pidl( ILClone( other.m_pidl ) ),
    m_name( other.m_name ),
    m_size( other.m_size ),
    m_modified( other.m_modified ),
    m_icon( other.m_icon ),
    m_attributes( other.m_attributes ),
    m_state( other.m_state )
{
}

ShellItemPrivate::~ShellItemPrivate()
{
    if ( m_pidl ) {
        CoTaskMemFree( m_pidl );
        m_pidl = NULL;
    }
}

ShellItem::ShellItem() :
    d( new ShellItemPrivate() )
{
}

ShellItem::~ShellItem()
{
}

ShellItem::ShellItem( const ShellItem& other ) :
    d( other.d )
{
}

ShellItem& ShellItem::operator =( const ShellItem& other )
{
    d = other.d;
    return *this;
}

bool ShellItem::isValid() const
{
    return d->m_pidl != NULL;
}

QString ShellItem::name() const
{
    return d->m_name;
}

qint64 ShellItem::size() const
{
    return d->m_size;
}

QDateTime ShellItem::lastModified() const
{
    return d->m_modified;
}

QPixmap ShellItem::icon() const
{
    return d->m_icon;
}

ShellItem::Attributes ShellItem::attributes() const
{
    return d->m_attributes;
}

ShellItem::State ShellItem::state() const
{
    return d->m_state;
}

void ShellItem::setSelected( bool selected )
{
    if ( selected )
        d->m_state |= IsSelected;
    else
        d->m_state &= ~IsSelected;
}

bool ShellItem::isSelected() const
{
    return d->m_state & IsSelected;
}

bool operator ==( const ShellItem& lhs, const ShellItem& rhs )
{
    return ILIsEqual( lhs.d->m_pidl, rhs.d->m_pidl );
}

bool operator !=( const ShellItem& lhs, const ShellItem& rhs )
{
    return !( lhs == rhs );
}
