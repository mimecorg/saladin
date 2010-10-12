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

#include "folderitemdelegate.h"

FolderItemDelegate::FolderItemDelegate( QObject* parent ) : QStyledItemDelegate( parent )
{
}

FolderItemDelegate::~FolderItemDelegate()
{
}

void FolderItemDelegate::updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/ ) const
{
    editor->setGeometry( option.rect.adjusted( 21, 0, 0, 0 ) );
}

bool FolderItemDelegate::eventFilter( QObject* object, QEvent* e )
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>( e );
        switch ( keyEvent->key() ) {
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                return false;
        }
    }

    return QStyledItemDelegate::eventFilter( object, e );
}
