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

#include "imagelabel.h"

ImageLabel::ImageLabel( QWidget* parent ) : QWidget( parent ),
    m_zoom( 1.0 ),
    m_black( false )
{
    setAttribute( Qt::WA_OpaquePaintEvent );

    setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
}

ImageLabel::~ImageLabel()
{
}

void ImageLabel::setImage( const QImage& image )
{
    m_image = image;
    updateGeometry();
    update();
}

void ImageLabel::setZoom( double zoom )
{
    m_zoom = zoom;
    if ( zoom < 0.0 )
        setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    else
        setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    updateGeometry();
    update();

    emit zoomChanged();
}

void ImageLabel::setBlackBackground( bool black )
{
    m_black = black;
    update();
}

double ImageLabel::actualZoom() const
{
    if ( m_zoom < 0.0 ) {
        double sx = double( width() ) / m_image.width();
        double sy = double( height() ) / m_image.height();
        return qMin( qMin( sx, sy ), 1.0 );
    } else {
        return m_zoom;
    }
}

QSize ImageLabel::sizeHint() const
{
    if ( m_zoom < 0.0 )
        return QSize();
    else
        return m_zoom * m_image.size();
}

void ImageLabel::paintEvent( QPaintEvent* /*e*/ )
{
    QPainter painter( this );

    painter.fillRect( rect(), m_black ? Qt::black : Qt::white );

    if ( m_image.isNull() )
        return;

    painter.setRenderHint( QPainter::SmoothPixmapTransform );

    double zoom = actualZoom();

    QSize scaled = zoom * m_image.size();

    QRect target;
    target.setTop( ( height() - scaled.height() ) / 2 );
    target.setLeft( ( width() - scaled.width() ) / 2 );
    target.setSize( scaled );

    painter.drawImage( target, m_image );
}

void ImageLabel::wheelEvent( QWheelEvent* e )
{
    if ( e->modifiers() & Qt::ControlModifier ) {
        if ( e->delta() > 0 )
            emit zoomIn();
        else
            emit zoomOut();
        e->accept();
    } else {
        e->ignore();
    }
}

void ImageLabel::resizeEvent( QResizeEvent* e )
{
    QWidget::resizeEvent( e );

    if ( m_zoom < 0.0 )
        emit zoomChanged();
}
