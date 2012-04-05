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

#include "drivestripmanager.h"

#include "shell/shellfolder.h"
#include "shell/shelldropdata.h"

class ShellDriveLessThan
{
public:
    ShellDriveLessThan()
    {
    }

    bool operator()( const ShellDrive& drive1, const ShellDrive& drive2 )
    {
        return drive1.letter() < drive2.letter();
    }
};

DriveStripManager::DriveStripManager( QWidget* parent ) : QObject( parent ),
    m_dropData( NULL )
{
    m_computer = new ShellComputer( parent );

    m_drives = m_computer->listDrives();
    qSort( m_drives.begin(), m_drives.end(), ShellDriveLessThan() );

    connect( m_computer, SIGNAL( driveChanged( const ShellDrive& ) ), this, SLOT( driveChanged( const ShellDrive& ) ) );
    connect( m_computer, SIGNAL( computerUpdated() ), this, SLOT( computerUpdated() ) );

    m_computer->startWatching();
}

DriveStripManager::~DriveStripManager()
{
}

ShellDrive DriveStripManager::driveAt( int index ) const
{
    return m_drives.at( index );
}

ShellDrive DriveStripManager::driveAt( XmlUi::ToolStrip* strip, const QPoint& pos ) const
{
    QToolButton* button = qobject_cast<QToolButton*>( strip->childAt( pos ) );

    if ( button ) {
        for ( int i = 0; i < m_strips.count(); i++ ) {
            if ( m_strips.at( i ).m_strip == strip ) {
                int index = m_strips.at( i ).m_actions.indexOf( button->defaultAction() );
                if ( index >= 0 )
                    return m_drives.at( index );
                break;
            }
        }
    }

    return ShellDrive();
}

ShellDrive DriveStripManager::driveFromFolder( ShellFolder* folder ) const
{
    QString path = folder->path();

    if ( path.length() > 2 && path.at( 1 ) == QLatin1Char( ':' ) ) {
        foreach ( ShellDrive drive, m_drives ) {
            if ( path.at( 0 ) == QLatin1Char( drive.letter() ) )
                return drive;
        }
    }

    return ShellDrive();
}

void DriveStripManager::registerToolStrip( XmlUi::ToolStrip* strip, QObject* receiver, const char* member )
{
    StripInfo info;

    info.m_strip = strip;
    info.m_mapper = new QSignalMapper( this );

    connect( info.m_mapper, SIGNAL( mapped( int ) ), receiver, member );

    populateToolStrip( info );

    m_strips.append( info );

    strip->setAcceptDrops( true );
    strip->installEventFilter( this );
}

void DriveStripManager::populateToolStrip( StripInfo& info )
{
    info.m_strip->clearToolActions();

    foreach ( QAction* action, info.m_actions )
        delete action;
    info.m_actions.clear();

    for ( int i = 0; i < m_drives.count(); i++ ) {
        ShellDrive drive = m_drives.at( i );

        QAction* action = new QAction( this );
        action->setText( QString( "&%1 - %2" ).arg( (QChar)drive.letter(), drive.name().replace( "&", "&&" ) ) );
        action->setIconText( (QChar)drive.letter() );
        action->setToolTip( drive.name() );
        action->setIcon( drive.icon() );

        info.m_mapper->setMapping( action, i );
        connect( action, SIGNAL( triggered() ), info.m_mapper, SLOT( map() ) );

        info.m_strip->addToolAction( action );

        info.m_actions.append( action );
    }
}

void DriveStripManager::driveChanged( const ShellDrive& drive )
{
    int index = -1;
    for ( int i = 0; i < m_drives.count(); i++ ) {
        if ( m_drives[ i ].letter() == drive.letter() ) {
            m_drives[ i ] = drive;
            index = i;
            break;
        }
    }

    if ( index < 0 )
        return;

    for ( int i = 0; i < m_strips.count(); i++ ) {
        QAction* action = m_strips.at( i ).m_actions.at( index );
        action->setToolTip( drive.name() );
        action->setIcon( drive.icon() );
    }
}

void DriveStripManager::computerUpdated()
{
    m_drives = m_computer->listDrives();
    qSort( m_drives.begin(), m_drives.end(), ShellDriveLessThan() );

    for ( int i = 0; i < m_strips.count(); i++ )
        populateToolStrip( m_strips[ i ] );
}

void DriveStripManager::showDrivesMenu( XmlUi::ToolStrip* strip )
{
    for ( int i = 0; i < m_strips.count(); i++ ) {
        if ( m_strips.at( i ).m_strip == strip ) {
            QMenu::exec( m_strips.at( i ).m_actions, strip->mapToGlobal( strip->geometry().bottomLeft() ) );
            break;
        }
    }
}

bool DriveStripManager::eventFilter( QObject* watched, QEvent* e )
{
    XmlUi::ToolStrip* strip = qobject_cast<XmlUi::ToolStrip*>( watched );
    if ( strip ) {
        switch ( e->type() ) {
            case QEvent::DragEnter:
                return stripDragEnterEvent( strip, static_cast<QDragEnterEvent*>( e ) );
            case QEvent::DragMove:
                return stripDragMoveEvent( strip, static_cast<QDragMoveEvent*>( e ) );
            case QEvent::DragLeave:
                return stripDragLeaveEvent( strip, static_cast<QDragLeaveEvent*>( e ) );
            case QEvent::Drop:
                return stripDropEvent( strip, static_cast<QDropEvent*>( e ) );
            default:
                break;
        }
    }

    return false;
}

bool DriveStripManager::stripDragEnterEvent( XmlUi::ToolStrip* strip, QDragEnterEvent* e )
{
    m_dropData = new ShellDropData( e, m_computer, strip );

    e->accept();

    return true;
}

bool DriveStripManager::stripDragMoveEvent( XmlUi::ToolStrip* strip, QDragMoveEvent* e )
{
    dragDropHelper( strip, e, false );

    return true;
}

bool DriveStripManager::stripDragLeaveEvent( XmlUi::ToolStrip* /*strip*/, QDragLeaveEvent* /*e*/ )
{
    delete m_dropData;
    m_dropData = NULL;

    return true;
}

bool DriveStripManager::stripDropEvent( XmlUi::ToolStrip* strip, QDropEvent* e )
{
    dragDropHelper( strip, e, true );

    delete m_dropData;
    m_dropData = NULL;

    return true;
}

bool DriveStripManager::dragDropHelper( XmlUi::ToolStrip* strip, QDropEvent* e, bool doDrop )
{
    if ( !m_dropData || !m_dropData->isValid() )
        return false;

    bool result = false;

    ShellDrive drive = driveAt( strip, e->pos() );

    if ( drive.isValid() )
        result = m_dropData->dragToDrive( e, drive );

    if ( result && doDrop )
        result = m_dropData->drop();

    if ( result ) {
        e->setDropAction( m_dropData->dropAction() );
        e->accept();
    } else {
        e->ignore();
    }

    return result;
}
