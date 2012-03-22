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

#include "elidedlabel.h"

#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

ElidedLabel::ElidedLabel( QWidget* parent ) : QLabel( parent ),
    m_lastWidth( 0 )
{
    setMinimumWidth( fontMetrics().width( "..." ) );
}

ElidedLabel::~ElidedLabel()
{
}

void ElidedLabel::paintEvent( QPaintEvent* /*e*/ )
{
    QPainter painter( this );
    drawFrame( &painter );

    QRect cr = contentsRect();
    cr.adjust( margin(), margin(), -margin(), -margin() );

    QString fullText = text();

    if ( fullText != m_lastText || cr.width() != m_lastWidth ) {
        m_elidedText = fontMetrics().elidedText( fullText, Qt::ElideRight, cr.width() );
        m_lastText = fullText;
        m_lastWidth = cr.width();
    }

    QStyleOption opt;
    opt.initFrom( this );

    style()->drawItemText( &painter, cr, alignment(), opt.palette, isEnabled(), m_elidedText, foregroundRole() );
}
