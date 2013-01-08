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

#ifndef SHELLITEM_H
#define SHELLITEM_H

#include <QSharedData>

class ShellItemPrivate;

class ShellItem
{
public:
    enum Attribute
    {
        // shell attributes
        Folder = 1,
        Stream = 2,
        FileSystem = 4,
        CanRename = 8,
        // file system attributes
        ReadOnly = 0x100,
        Hidden = 0x200,
        System = 0x400,
        Directory = 0x800,
        Archive = 0x1000,
        Compressed = 0x2000,
        Encrypted = 0x4000,
        ReparsePoint = 0x8000,
    };
    Q_DECLARE_FLAGS( Attributes, Attribute );

    enum StateFlag
    {
        HasProperties = 1,
        HasExtractedIcon = 2,
        HasCalculatedSize = 4,
        IsSelected = 8
    };
    Q_DECLARE_FLAGS( State, StateFlag );

public:
    ShellItem();
    ~ShellItem();

    ShellItem( const ShellItem& other );
    ShellItem& operator =( const ShellItem& other );

public:
    bool isValid() const;

    QString name() const;
    qint64 size() const;
    QDateTime lastModified() const;
    QPixmap icon() const;

    Attributes attributes() const;
    State state() const;

    void setSelected( bool selected );
    bool isSelected() const;

public:
    friend bool operator ==( const ShellItem& lhs, const ShellItem& rhs );
    friend bool operator !=( const ShellItem& lhs, const ShellItem& rhs );

private:
    QSharedDataPointer<ShellItemPrivate> d;

    friend class ShellFolder;
    friend class ShellFolderPrivate;
    friend class ShellSelection;
    friend class ShellSelectionPrivate;
    friend class ShellComputer;
    friend class ShellDropData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( ShellItem::Attributes )
Q_DECLARE_OPERATORS_FOR_FLAGS( ShellItem::State )

Q_DECLARE_TYPEINFO( ShellItem, Q_MOVABLE_TYPE );

#endif
