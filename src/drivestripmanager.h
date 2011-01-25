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

#ifndef DRIVESTRIPMANAGER_H
#define DRIVESTRIPMANAGER_H

#include "shell/shelldrive.h"
#include "shell/shellcomputer.h"
#include "xmlui/toolstrip.h"

#include <QObject>

class DriveStripManager : public QObject
{
    Q_OBJECT
public:
    explicit DriveStripManager( QWidget* parent );
    ~DriveStripManager();

public:
    ShellComputer* computer() const { return m_computer; }

    ShellDrive driveAt( int index ) const;
    ShellDrive driveAt( XmlUi::ToolStrip* strip, const QPoint& pos ) const;

    ShellDrive driveFromFolder( ShellFolder* folder ) const;

    void registerToolStrip( XmlUi::ToolStrip* strip, QObject* receiver, const char* member );

    void showDrivesMenu( XmlUi::ToolStrip* strip );

public: // overrides
    bool eventFilter( QObject* watched, QEvent* e );

private slots:
    void driveChanged( const ShellDrive& drive );
    void computerUpdated();

private:
    struct StripInfo
    {
        XmlUi::ToolStrip* m_strip;
        QSignalMapper* m_mapper;
        QList<QAction*> m_actions;
    };

private:
    void populateToolStrip( StripInfo& info );

    bool stripDragEnterEvent( XmlUi::ToolStrip* strip, QDragEnterEvent* e );
    bool stripDragMoveEvent( XmlUi::ToolStrip* strip, QDragMoveEvent* e );
    bool stripDragLeaveEvent( XmlUi::ToolStrip* strip, QDragLeaveEvent* e );
    bool stripDropEvent( XmlUi::ToolStrip* strip, QDropEvent* e );

    bool dragDropHelper( XmlUi::ToolStrip* strip, QDropEvent* e, bool doDrop );

private:
    ShellComputer* m_computer;
    QList<ShellDrive> m_drives;

    QList<StripInfo> m_strips;

    ShellDropData* m_dropData;

    friend class ShellDriveLessThan;
};

#endif
