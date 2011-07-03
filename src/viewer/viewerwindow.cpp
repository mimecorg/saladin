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

#include "mainwindow.h"
#include "viewer/viewmanager.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

ViewerWindow::ViewerWindow() : QMainWindow(),
    m_view( NULL )
{
    setAttribute( Qt::WA_DeleteOnClose, true );

    QAction* action;

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

    setStatusBar( new QStatusBar( this ) );
}

ViewerWindow::~ViewerWindow()
{
}

void ViewerWindow::setView( View* view )
{
    builder()->supressUpdate();

    delete m_view;
    m_view = view;

    builder()->addClient( view );

    setCentralWidget( view->mainWidget() );

    View::Type type = view->type();

    action( "switchToText" )->setChecked( type == View::Text );
    action( "switchToBinary" )->setChecked( type == View::Binary );
    action( "switchToImage" )->setChecked( type == View::Image );

    builder()->resumeUpdate();
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
