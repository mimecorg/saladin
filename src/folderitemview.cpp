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

#include "folderitemview.h"

FolderItemView::FolderItemView( QWidget* parent ) : QTreeView( parent )
{
}

FolderItemView::~FolderItemView()
{
}

bool FolderItemView::isEditing() const
{
    return state() == EditingState;
}

QModelIndex FolderItemView::movePageUp()
{
    return moveCursor( QAbstractItemView::MovePageUp, Qt::NoModifier );
}

QModelIndex FolderItemView::movePageDown()
{
    return moveCursor( QAbstractItemView::MovePageDown, Qt::NoModifier );
}

void FolderItemView::drawRow( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QTreeView::drawRow( painter, option, index );

    if ( hasFocus() && index.row() == currentIndex().row() && state() != QAbstractItemView::EditingState ) {
    	QRect rect( -header()->offset(), option.rect.top(), header()->length(), option.rect.height() );
        rect = style()->visualRect( layoutDirection(), viewport()->rect(), rect );

        painter->setPen( QColor( 0, 0, 255 ) );
        painter->drawRect( rect.adjusted( 0, 0, -1, -1 ) );
    }
}

void FolderItemView::setAnchor( const QModelIndex& index )
{
    m_anchor = index;
}

QModelIndex FolderItemView::anchor() const
{
    return m_anchor;
}

void FolderItemView::currentChanged( const QModelIndex& current, const QModelIndex& previous )
{
    QTreeView::currentChanged( current, previous );
    m_anchor = QPersistentModelIndex();
}
