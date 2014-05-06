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

#ifndef FOLDERITEMMODEL_H
#define FOLDERITEMMODEL_H

#include "shell/shellitem.h"
#include "shell/shellfolder.h"

#include <QMainWindow>

class FolderItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Columns
    {
        Column_Name,
        Column_Size,
        Column_LastModified,
        Column_Attributes
    };

public:
    explicit FolderItemModel( QObject* parent );
    ~FolderItemModel();

public:
    void setIncludeHidden( bool on );
    bool includeHidden() const { return m_includeHidden; }

    void setFolder( ShellFolder* folder );
    ShellFolder* folder() const { return m_folder; }

    const QList<ShellItem>& items() const { return m_items; }

    ShellItem itemAt( const QModelIndex& index ) const;
    QModelIndex indexOf( const ShellItem& item, int column = 0 ) const;

    bool isParentFolder( const QModelIndex& index ) const;

    void setItemSelected( const QModelIndex& index, bool selected );
    bool isItemSelected( const QModelIndex& index ) const;

    void toggleItemSelected( const QModelIndex& index );

    QList<ShellItem> selectedItems() const;

    void selectAll();
    void unselectAll();
    void invertSelection();

    void calculateSize( const QModelIndex& index );
    void calculateSizeSelected();

    qint64 totalItemsSize() const;
    qint64 selectedItemsSize() const;

    int totalItemsCount() const;
    int selectedItemsCount() const;

    void compareWith( const QList<ShellItem>& items );

    void refresh();

    void setGotoItemName( const QString& name );
    const QModelIndex gotoItemIndex();

public: // overrides
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex& index ) const;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

    Qt::ItemFlags flags( const QModelIndex& index ) const;

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

private slots:
    void extractNextIcon();

    void itemChanged( const ItemChange& change );
    void folderUpdated();

    void applyChanges();

private:
    int indexToRow( const QModelIndex& index ) const;
    QModelIndex rowToIndex( int row, int column ) const;

    void resort();

    bool shellItemLessThan( const ShellItem& item1, const ShellItem& item2 );

    QStringList storeIndexes( const QModelIndexList& indexes );
    QModelIndexList restoreIndexes( const QStringList& names, const QModelIndexList& oldIndexes );

private:
    bool m_includeHidden;

    ShellFolder* m_folder;
    QList<ShellItem> m_items;

    bool m_hasParent;

    int m_sortColumn;
    Qt::SortOrder m_sortOrder;

    QList<int> m_extractQueue;

    QList<ItemChange> m_changes;
    bool m_pendingRefresh;
    bool m_pendingUpdate;

    QString m_gotoItemName;

    friend class ShellItemLessThan;
};

#endif
