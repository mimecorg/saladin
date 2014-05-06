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

#ifndef SHELLCOMPUTER_H
#define SHELLCOMPUTER_H

#include "shelldrive.h"
#include "shellselection.h"

#include <QObject>

class ShellFolder;
class ShellComputerPrivate;

class ShellComputer : public QObject
{
    Q_OBJECT
public:
    explicit ShellComputer( QWidget* parent );
    ~ShellComputer();

public:
    QList<ShellDrive> listDrives();

    ShellFolder* openRootFolder( const ShellDrive& drive );

    ShellSelection::MenuCommand showContextMenu( const ShellDrive& drive, const QPoint& pos, ShellSelection::Flags flags );

    bool startWatching();

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>( QObject::parent() ); }

signals:
    void driveChanged( const ShellDrive& drive );
    void computerUpdated();

private slots:
    void changeNotify( int eventType, void* arg1, void* arg2 );

private:
    ShellComputerPrivate* d;

    friend class ShellDropData;
};

#endif
