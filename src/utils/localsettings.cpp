/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2012 Michał Męciński
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

#include "localsettings.h"
#include "dataserializer.h"

LocalSettings::LocalSettings( const QString& path, QObject* parent ) : QObject( parent ),
    m_path( path )
{
    load();
}

LocalSettings::~LocalSettings()
{
    save();
}

bool LocalSettings::contains( const QString& key ) const
{
    return m_data.contains( key );
}

void LocalSettings::setValue( const QString& key, const QVariant& value )
{
    m_data.insert( key, value );
}

QVariant LocalSettings::value( const QString& key, const QVariant& defaultValue /*= QVariant()*/ ) const
{
    return m_data.value( key, defaultValue );
}

void LocalSettings::load()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForReading() )
        return;

    serializer.stream() >> m_data;
}

void LocalSettings::save()
{
    DataSerializer serializer( m_path );

    if ( !serializer.openForWriting() )
        return;

    serializer.stream() << m_data;

    emit settingsChanged();
}
