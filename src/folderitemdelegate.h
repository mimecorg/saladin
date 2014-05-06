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

#ifndef FOLDERITEMDELEGATE_H
#define FOLDERITEMDELEGATE_H

#include "shell/shellitem.h"
#include "shell/shellfolder.h"

#include <QStyledItemDelegate>

class FolderItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FolderItemDelegate( QObject* parent );
    ~FolderItemDelegate();

public: // overrides
    void updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const;

    bool eventFilter( QObject* object, QEvent* e );
};

#endif
