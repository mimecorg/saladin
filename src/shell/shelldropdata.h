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

#ifndef SHELLDROPDATA_H
#define SHELLDROPDATA_H

#include <QObject>

class ShellDropDataPrivate;
class ShellFolder;
class ShellItem;

class ShellDropData : public QObject
{
    Q_OBJECT
public:
    ShellDropData( QDropEvent* e, QWidget* parent );
    ~ShellDropData();

public:
    bool isValid() const;

    Qt::DropAction dropAction() const;

    bool dragMove( QDragMoveEvent* e, ShellFolder* folder );
    bool dragMove( QDragMoveEvent* e, ShellFolder* folder, const ShellItem& item );

    bool drop( QDropEvent* e, ShellFolder* folder );
    bool drop( QDropEvent* e, ShellFolder* folder, const ShellItem& item );

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>( QObject::parent() ); }

private:
    ShellDropDataPrivate* d;
};

#endif
