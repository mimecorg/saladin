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

#ifndef FOLDERITEMVIEW_H
#define FOLDERITEMVIEW_H

#include <QTreeView>

class FolderItemView : public QTreeView
{
    Q_OBJECT
public:
    explicit FolderItemView( QWidget* parent );
    ~FolderItemView();

public:
    bool isEditing() const;

    QModelIndex movePageUp();
    QModelIndex movePageDown();

    void setAnchor( const QModelIndex& index );
    QModelIndex anchor() const;

    bool isDragging() const;
    void setDragging( bool dragging );

    void highlightDropItem( const QModelIndex& index );

    void checkAutoScroll( const QPoint& pos );

protected: // overrides
    void drawRow( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;

    void currentChanged( const QModelIndex& current, const QModelIndex& previous );

private:
    QPersistentModelIndex m_anchor;
};

#endif
