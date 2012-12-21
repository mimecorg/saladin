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

#ifndef SEARCHITEMMODEL_H
#define SEARCHITEMMODEL_H

#include "shell/shellitem.h"
#include "shell/shellfolder.h"
#include "shell/shellpidl.h"

class SearchItemModel : public QAbstractItemModel
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
    explicit SearchItemModel( QObject* parent );
    ~SearchItemModel();

public:
    void startSearch( ShellFolder* folder, const QString& pattern );
    void abortSearch();

    bool isSearching() const;

    ShellItem itemAt( const QModelIndex& index ) const;
    QString pathAt( const QModelIndex& index ) const;

public: // overrides
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex& index ) const;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    Qt::ItemFlags flags( const QModelIndex& index ) const;

signals:
    void searchCompleted();

private slots:
    void scanNextFolder();

    void extractNextIcon();

private:
    void clear();

    bool scanFolder( const QString& prefix, ShellFolder* folder );

private:
    struct FoundItem
    {
        ShellItem m_item;
        ShellFolder* m_folder;
        QString m_prefix;
    };

    struct PendingFolder
    {
        ShellPidl m_pidl;
        QString m_prefix;
    };

    friend bool operator <( const PendingFolder& folder1, const PendingFolder& folder2 );

private:
    QList<ShellFolder*> m_folders;

    QList<FoundItem> m_items;

    QList<PendingFolder> m_pendingFolders;

    QList<QRegExp> m_filters;

    QList<int> m_extractQueue;
};

class SearchProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SearchProxyModel( QObject* parent );
    ~SearchProxyModel();

protected: // overrides
    bool lessThan( const QModelIndex& index1, const QModelIndex& index2 ) const;
};

#endif
