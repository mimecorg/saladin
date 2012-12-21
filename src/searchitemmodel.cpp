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

#include "searchitemmodel.h"

#include "utils/iconloader.h"

SearchItemModel::SearchItemModel( QObject* parent ) : QAbstractItemModel( parent )
{
}

SearchItemModel::~SearchItemModel()
{
    clear();
}

void SearchItemModel::clear()
{
    qDeleteAll( m_folders );
    m_folders.clear();

    m_items.clear();

    m_pendingFolders.clear();

    m_filters.clear();

    m_extractQueue.clear();
}

void SearchItemModel::startSearch( ShellFolder* folder, const QString& pattern )
{
    clear();

    reset();

    QStringList parts = pattern.split( QLatin1Char( ';' ), QString::SkipEmptyParts );

    foreach ( QString part, parts )
        m_filters.append( QRegExp( part, Qt::CaseInsensitive, QRegExp::Wildcard ) );

    scanFolder( QString(), folder );

    if ( !m_pendingFolders.isEmpty() )
        QTimer::singleShot( 0, this, SLOT( scanNextFolder() ) );
    else
        emit searchCompleted();
}

void SearchItemModel::abortSearch()
{
    m_pendingFolders.clear();
}

bool SearchItemModel::isSearching() const
{
    return !m_pendingFolders.isEmpty();
}

void SearchItemModel::scanNextFolder()
{
    if ( m_pendingFolders.isEmpty() )
        return;

    PendingFolder pendingFolder = m_pendingFolders.takeFirst();

    ShellFolder* folder = new ShellFolder( pendingFolder.m_pidl, qobject_cast<QWidget*>( QObject::parent() ) );

    if ( scanFolder( pendingFolder.m_prefix, folder ) )
        m_folders.append( folder );
    else
        delete folder;

    if ( !m_pendingFolders.isEmpty() )
        QTimer::singleShot( 0, this, SLOT( scanNextFolder() ) );
    else
        emit searchCompleted();
}

bool operator <( const SearchItemModel::PendingFolder& folder1, const SearchItemModel::PendingFolder& folder2 )
{
    return StrCmpLogicalW( folder1.m_prefix.utf16(), folder2.m_prefix.utf16() ) < 0;
}

bool SearchItemModel::scanFolder( const QString& prefix, ShellFolder* folder )
{
    QList<ShellItem> items = folder->listItems( 0 );

    QList<PendingFolder> pendingFolders;
    QList<FoundItem> foundItems;

    foreach ( ShellItem item, items ) {
        if ( item.attributes().testFlag( ShellItem::Directory ) ) {
            PendingFolder pendingFolder;
            pendingFolder.m_pidl = folder->itemPidl( item );
            pendingFolder.m_prefix = prefix + item.name() + QLatin1Char( '\\' );
            pendingFolders.append( pendingFolder );
        } else {
            foreach ( QRegExp filter, m_filters ) {
                if ( filter.exactMatch( item.name() ) ) {
                    FoundItem foundItem;
                    foundItem.m_item = item;
                    foundItem.m_folder = folder;
                    foundItem.m_prefix = prefix;
                    foundItems.append( foundItem );
                    break;
                }
            }
        }
    }

    if ( !pendingFolders.isEmpty() ) {
        qSort( pendingFolders );
        m_pendingFolders = pendingFolders + m_pendingFolders;
    }

    if ( foundItems.isEmpty() )
        return false;

    beginInsertRows( QModelIndex(), m_items.count(), m_items.count() + foundItems.count() - 1 );

    m_items.append( foundItems );

    endInsertRows();

    return true;
}

void SearchItemModel::extractNextIcon()
{
    if ( m_extractQueue.isEmpty() )
        return;

    int row = m_extractQueue.takeFirst();

    m_items.at( row ).m_folder->extractIcon( m_items[ row ].m_item );

    QModelIndex index = createIndex( row, 0 );
    emit dataChanged( index, index );

    if ( !m_extractQueue.isEmpty() )
        QTimer::singleShot( 0, this, SLOT( extractNextIcon() ) );
}

ShellItem SearchItemModel::itemAt( const QModelIndex& index ) const
{
    int row = index.row();
    if ( row < 0 )
        return ShellItem();
    return m_items.at( row ).m_item;
}

QString SearchItemModel::pathAt( const QModelIndex& index ) const
{
    int row = index.row();
    if ( row < 0 )
        return QString();
    return m_items.at( row ).m_prefix+ m_items.at( row ).m_item.name();
}

int SearchItemModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return 4;
    return 0;
}

QVariant SearchItemModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        switch ( section ) {
            case Column_Name:
                return tr( "Name" );
            case Column_Size:
                return tr( "Size" );
            case Column_LastModified:
                return tr( "Date" );
            case Column_Attributes:
                return tr( "Attributes" );
        }
    }
    return QVariant();
}

int SearchItemModel::rowCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return m_items.count();
    return 0;
}

QModelIndex SearchItemModel::index( int row, int column, const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return createIndex( row, column );
    return QModelIndex();
}

QModelIndex SearchItemModel::parent( const QModelIndex& /*index*/ ) const
{
    return QModelIndex();
}

static QString formatAttributes( ShellItem::Attributes attributes )
{
    char result[ 8 ];
    result[ 0 ] = attributes.testFlag( ShellItem::Directory ) ? 'd' : '-';
    result[ 1 ] = attributes.testFlag( ShellItem::ReadOnly ) ? 'r' : '-';
    result[ 2 ] = attributes.testFlag( ShellItem::Archive ) ? 'a' : '-';
    result[ 3 ] = attributes.testFlag( ShellItem::Hidden ) ? 'h' : '-';
    result[ 4 ] = attributes.testFlag( ShellItem::System ) ? 's' : '-';
    result[ 5 ] = attributes.testFlag( ShellItem::Compressed ) ? 'c' : '-';
    result[ 6 ] = attributes.testFlag( ShellItem::Encrypted ) ? 'e' : '-';
    result[ 7 ] = 0;
    return result;
}

QVariant SearchItemModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    if ( role == Qt::DisplayRole || role == Qt::EditRole ) {
        int row = index.row();
        if ( row >= 0 ) {
            const ShellItem& item = m_items.at( row ).m_item;

            switch ( index.column() ) {
                case Column_Name:
                    return m_items.at( row ).m_prefix + item.name();
                case Column_Size:
                    if ( item.state().testFlag( ShellItem::HasProperties ) )
                        return QLocale::system().toString( item.size() ) + ' ';
                    break;
                case Column_LastModified:
                    if ( item.state().testFlag( ShellItem::HasProperties ) )
                        return item.lastModified().toString( "yyyy-MM-dd hh:mm" );
                    break;
                case Column_Attributes:
                    if ( item.state().testFlag( ShellItem::HasProperties ) )
                        return formatAttributes( item.attributes() );
                    break;
            }
        }

        return QVariant();
    }

    if ( role == Qt::DecorationRole && index.column() == Column_Name ) {
        int row = index.row();
        if ( row >= 0 ) {
            if ( !m_items.at( row ).m_item.state().testFlag( ShellItem::HasExtractedIcon ) && !m_extractQueue.contains( row ) ) {
                SearchItemModel* that = const_cast<SearchItemModel*>( this );
                if ( m_extractQueue.isEmpty() )
                    QTimer::singleShot( 0, that, SLOT( extractNextIcon() ) );
                that->m_extractQueue.append( row );
            }

            return m_items.at( row ).m_item.icon();
        }
    }

    if ( role == Qt::ToolTipRole && index.column() == Column_Name ) {
        int row = index.row();
        if ( row >= 0 )
            return m_items.at( row ).m_folder->toolTip( m_items.at( row ).m_item );
    }

    if ( role == Qt::TextAlignmentRole && index.column() == Column_Size )
        return (int)( Qt::AlignRight | Qt::AlignVCenter );

    if ( role == Qt::ForegroundRole ) {
        int row = index.row();
        if ( row >= 0 ) {
            const ShellItem& item = m_items.at( row ).m_item;

            if ( item.attributes().testFlag( ShellItem::Hidden ) )
                return QColor( 128, 128, 128 );

            if ( item.attributes().testFlag( ShellItem::Compressed ) )
                return QColor( 0, 0, 128 );
        }
    }

    return QVariant();
}

Qt::ItemFlags SearchItemModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    return Qt::ItemIsEnabled;
}

SearchProxyModel::SearchProxyModel( QObject* parent ) : QSortFilterProxyModel( parent )
{
}

SearchProxyModel::~SearchProxyModel()
{
}

bool SearchProxyModel::lessThan( const QModelIndex& index1, const QModelIndex& index2 ) const
{
    SearchItemModel* source = static_cast<SearchItemModel*>( sourceModel() );

    int result = 0;

    if ( sortColumn() != SearchItemModel::Column_Name ) {
        ShellItem item1 = source->itemAt( index1 );
        ShellItem item2 = source->itemAt( index2 );

        switch ( sortColumn() ) {
            case SearchItemModel::Column_Size:
                if ( item1.size() < item2.size() )
                    result = -1;
                else if ( item2.size() < item1.size() )
                    result = 1;
                break;
            case SearchItemModel::Column_LastModified:
                if ( item1.lastModified() < item2.lastModified() )
                    result = -1;
                else if ( item2.lastModified() < item1.lastModified() )
                    result = 1;
                break;
            case SearchItemModel::Column_Attributes:
                result = QString::localeAwareCompare( formatAttributes( item1.attributes() ), formatAttributes( item2.attributes() ) );
                break;
        }
    }

    if ( result == 0 ) {
        QString path1 = source->pathAt( index1 );
        QString path2 = source->pathAt( index2 );

        result = StrCmpLogicalW( path1.utf16(), path2.utf16() );
    }

    return result < 0;
}