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

#include "iconcache_p.h"

#include <shlobj.h>

static QPixmap loadIcon( const wchar_t* pattern, int attributes )
{
    SHFILEINFO info;
    DWORD_PTR list = SHGetFileInfo( pattern, attributes, &info, sizeof( info ),
        SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES );

    QPixmap icon;

    if ( list != 0 ) {
        icon = QPixmap::fromWinHICON( info.hIcon );
        DestroyIcon( info.hIcon );
    }

    return icon;
}

class IconCacheGlobal
{
public:
    IconCacheGlobal();
    ~IconCacheGlobal();

public:
    QPixmap m_fileIcon;
    QPixmap m_directoryIcon;

    QMap<QString, QPixmap> m_fileIcons;
};

Q_GLOBAL_STATIC( IconCacheGlobal, iconCacheGlobal )

IconCacheGlobal::IconCacheGlobal()
{
    m_fileIcon = loadIcon( L"*.*", FILE_ATTRIBUTE_NORMAL );
    m_directoryIcon = loadIcon( L"*.*", FILE_ATTRIBUTE_DIRECTORY );
}

IconCacheGlobal::~IconCacheGlobal()
{
}

QPixmap IconCache::fileIcon()
{
    return iconCacheGlobal()->m_fileIcon;
}

QPixmap IconCache::directoryIcon()
{
    return iconCacheGlobal()->m_directoryIcon;
}

QPixmap IconCache::loadFileIcon( const QString& extension )
{
    IconCacheGlobal* g = iconCacheGlobal();

    QMap<QString, QPixmap>::const_iterator it = g->m_fileIcons.find( extension );
    if ( it != g->m_fileIcons.end() )
        return it.value();

    QString pattern = "*." + extension;

    QPixmap icon = loadIcon( pattern.utf16(), FILE_ATTRIBUTE_NORMAL );

    g->m_fileIcons.insert( extension, icon );

    return icon;
}
