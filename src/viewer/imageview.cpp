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

ImageView::ImageView( QObject* parent, QWidget* parentWidget ) : View( parent )
{
    QScrollArea* scroll = new QScrollArea( parentWidget );
    scroll->setWidgetResizable( true );

    QPalette scrollPalette = parentWidget->palette();
    scrollPalette.setColor( QPalette::Window, QColor::fromRgb( 255, 255, 255 ) );
    scroll->setPalette( scrollPalette );

    m_label = new QLabel( scroll );
    m_label->setAlignment( Qt::AlignCenter );

    scroll->setWidget( m_label );

    setMainWidget( scroll );

    setStatus( tr( "Image" ) );
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
}
