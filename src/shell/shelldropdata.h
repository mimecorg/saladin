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

#ifndef SHELLDROPDATA_H
#define SHELLDROPDATA_H

#include <QObject>

class ShellDropDataPrivate;
class ShellFolder;
class ShellComputer;
class ShellItem;
class ShellDrive;

class ShellDropData : public QObject
{
    Q_OBJECT
public:
    ShellDropData( QDropEvent* e, ShellFolder* folder, QWidget* parent );
    ShellDropData( QDropEvent* e, ShellComputer* computer, QWidget* parent );
    ~ShellDropData();

public:
    bool isValid() const;

    Qt::DropAction dropAction() const;

    bool dragToFolder( QDropEvent* e );
    bool dragToParent( QDropEvent* e );
    bool dragToItem( QDropEvent* e, const ShellItem& item );
    bool dragToDrive( QDropEvent* e, const ShellDrive& drive );

    bool drop();

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>( QObject::parent() ); }

private:
    ShellDropDataPrivate* d;

    friend class ShellSelection;
};

#endif
