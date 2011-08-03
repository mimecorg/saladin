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

#include "viewerwindow.h"

#include "application.h"
#include "mainwindow.h"
#include "viewer/viewmanager.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

ViewerWindow::ViewerWindow() : QMainWindow(),
    m_view( NULL )
{
    setAttribute( Qt::WA_DeleteOnClose, true );

    QAction* action;

    action = new QAction( IconLoader::icon( "refresh" ), tr( "Reload" ), this );
    action->setShortcut( Qt::CTRL + Qt::Key_R );
    connect( action, SIGNAL( triggered() ), this, SLOT( reload() ) );
    setAction( "reload", action );

    action = new QAction( IconLoader::icon( "type-text" ), tr( "Text" ), this );
    action->setShortcut( Qt::Key_1 );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( switchToText() ) );
    setAction( "switchToText", action );

    action = new QAction( IconLoader::icon( "type-binary" ), tr( "Binary" ), this );
    action->setShortcut( Qt::Key_2 );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( switchToBinary() ) );
    setAction( "switchToBinary", action );

    action = new QAction( IconLoader::icon( "type-image" ), tr( "Image" ), this );
    action->setShortcut( Qt::Key_3 );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( switchToImage() ) );
    setAction( "switchToImage", action );

    QShortcut* shortcut;

    shortcut = new QShortcut( Qt::Key_Escape, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( close() ) );

    loadXmlUiFile( ":/resources/viewerwindow.xml" );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    strip->setContentsMargins( 3, 3, 3, 3 );

    setMenuWidget( strip );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->registerToolStrip( "stripMain", strip );
    builder->addClient( this );

    QStatusBar* status = new QStatusBar( this );
    setStatusBar( status );

    m_statusLabel = new QLabel( status );
    m_statusLabel->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );
    status->addWidget( m_statusLabel, 1 );

    initializeGeometry();
}

ViewerWindow::~ViewerWindow()
{
    storeGeometry( false );
}

void ViewerWindow::setView( View* view )
{
    builder()->supressUpdate();

    delete m_view;
    m_view = view;

    builder()->addClient( view );

    setCentralWidget( view->mainWidget() );

    statusChanged( view->status() );

    connect( view, SIGNAL( statusChanged( const QString& ) ), this, SLOT( statusChanged( const QString& ) ) );

    View::Type type = view->type();

    action( "switchToText" )->setChecked( type == View::Text );
    action( "switchToBinary" )->setChecked( type == View::Binary );
    action( "switchToImage" )->setChecked( type == View::Image );

    builder()->resumeUpdate();
}

void ViewerWindow::reload()
{
    m_view->load();
}

void ViewerWindow::switchToText()
{
    if ( m_view->type() != View::Text )
        mainWindow->viewManager()->switchViewType( this, View::Text );
    else
        action( "switchToText" )->setChecked( true );
}

void ViewerWindow::switchToBinary()
{
    if ( m_view->type() != View::Binary )
        mainWindow->viewManager()->switchViewType( this, View::Binary );
    else
        action( "switchToBinary" )->setChecked( true );
}

void ViewerWindow::switchToImage()
{
    if ( m_view->type() != View::Image )
        mainWindow->viewManager()->switchViewType( this, View::Image );
    else
        action( "switchToImage" )->setChecked( true );
}

void ViewerWindow::statusChanged( const QString& status )
{
    m_statusLabel->setText( status );
}

void ViewerWindow::showEvent( QShowEvent* e )
{
    if ( !e->spontaneous() )
        storeGeometry( true );
}

void ViewerWindow::initializeGeometry()
{
    LocalSettings* settings = application->applicationSettings();

    if ( settings->contains( "ViewerWindowGeometry" ) ) {
        restoreGeometry( settings->value( "ViewerWindowGeometry" ).toByteArray() );

        if ( settings->value( "ViewerWindowOffset" ).toBool() ) {
            QPoint position = pos() + QPoint( 40, 40 );
            QRect available = QApplication::desktop()->availableGeometry( this );
            QRect frame = frameGeometry();
            if ( position.x() + frame.width() > available.right() )
                position.rx() = available.left();
            if ( position.y() + frame.height() > available.bottom() - 20 )
                position.ry() = available.top();
            move( position );
        }
    } else {
        QRect available = QApplication::desktop()->availableGeometry( this );
        resize( available.width() * 4 / 5, available.height() * 4 / 5 );
        setWindowState( Qt::WindowMaximized );
    }
}

void ViewerWindow::storeGeometry( bool offset )
{
    LocalSettings* settings = application->applicationSettings();

    QString geometryKey = QString( "%1Geometry" ).arg( m_view->metaObject()->className() );
    QString offsetKey = QString( "%1Offset" ).arg( m_view->metaObject()->className() );

    settings->setValue( "ViewerWindowGeometry", saveGeometry() );
    settings->setValue( "ViewerWindowOffset", offset );
}
