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

#ifndef SHELLSELECTION_H
#define SHELLSELECTION_H

#include <QObject>

class ShellFolder;
class ShellItem;
class ShellSelectionPrivate;

class ShellSelection : public QObject
{
    Q_OBJECT
public:
    enum TransferType
    {
        Copy,
        Move
    };

    enum Flag
    {
        ForceOverwrite = 1,
        ForceDelete = 2,
        DeletePermanently = 4,
        CanOpen = 8,
        CanRename = 16,
    };
    Q_DECLARE_FLAGS( Flags, Flag );

    enum MenuCommand
    {
        NoCommand,
        InternalCommand,
        Open,
        Rename
    };

public:
    ShellSelection( ShellFolder* folder, const QList<ShellItem>& items, QWidget* parent );
    ~ShellSelection();

public:
    bool canTransferTo( ShellFolder* targetFolder, TransferType type );
    bool transferTo( ShellFolder* targetFolder, TransferType type, Flags flags, QStringList newNames );

    bool canDragDropTo( ShellFolder* targetFolder, TransferType type );
    bool dragDropTo( ShellFolder* targetFolder, TransferType type );

    bool canDelete();
    bool deleteSelection( Flags flags );

    MenuCommand showContextMenu( const QPoint& pos, Flags flags );

    bool invokeCommand( const char* verb );

    bool doDragDrop();

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>( QObject::parent() ); }

private:
    ShellSelectionPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( ShellSelection::Flags )

#endif
