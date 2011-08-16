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

#include "imageview.h"

#include "utils/iconloader.h"
#include "xmlui/builder.h"

ImageView::ImageView( QObject* parent, QWidget* parentWidget ) : View( parent )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy" ), this );
    action->setShortcut( QKeySequence::Copy );
    connect( action, SIGNAL( triggered() ), this, SLOT( copy() ) );
    setAction( "copy", action );

    loadXmlUiFile( ":/resources/imageview.xml" );

    QWidget* main = new QWidget( parentWidget );
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setContentsMargins( 3, 0, 3, 0 );
    mainLayout->setSpacing( 0 );

    QScrollArea* scroll = new QScrollArea( main );
    scroll->setWidgetResizable( true );

    QPalette scrollPalette = parentWidget->palette();
    scrollPalette.setColor( QPalette::Window, QColor::fromRgb( 255, 255, 255 ) );
    scroll->setPalette( scrollPalette );

    m_label = new QLabel( scroll );
    m_label->setAlignment( Qt::AlignCenter );
    m_label->setContextMenuPolicy( Qt::CustomContextMenu );

    scroll->setWidget( m_label );

    connect( m_label, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( contextMenuRequested( const QPoint& ) ) );

    mainLayout->addWidget( scroll );

    main->setFocusProxy( scroll );

    setMainWidget( main );

    setStatus( tr( "Image" ) );

    updateActions();
}

ImageView::~ImageView()
{
}

View::Type ImageView::type() const
{
    return Image;
}

void ImageView::load()
{
    QString status = tr( "Image" );

    m_label->clear();

    if ( !format().isEmpty() ) {
        status += ", " + format().toUpper();

        QPixmap pixmap( path(), format().data() );

        if ( !pixmap.isNull() ) {
            m_label->setPixmap( pixmap );

            status += QString( " (%1 x %2)" ).arg( pixmap.width() ).arg( pixmap.height() );
        }
    }

    setStatus( status );

    updateActions();
}

void ImageView::updateActions()
{
    bool hasPixmap = m_label->pixmap() != NULL;

    action( "copy" )->setEnabled( hasPixmap );
}

void ImageView::copy()
{
    const QPixmap* pixmap = m_label->pixmap();
    if ( pixmap )
        QApplication::clipboard()->setPixmap( *pixmap );
}

void ImageView::contextMenuRequested( const QPoint& pos )
{
    QMenu* menu = builder()->contextMenu( "menuContext" );
    if ( menu )
        menu->popup( m_label->mapToGlobal( pos ) );
}
