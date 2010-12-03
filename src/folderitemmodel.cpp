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

#include "folderitemmodel.h"

#include "utils/iconloader.h"

class ShellItemLessThan
{
public:
    ShellItemLessThan( FolderItemModel* model ) :
        m_model( model )
    {
    }

    bool operator()( const ShellItem& item1, const ShellItem& item2 )
    {
        return m_model->shellItemLessThan( item1, item2 );
    }

private:
    FolderItemModel* m_model;
};

FolderItemModel::FolderItemModel( QObject* parent ) : QAbstractItemModel( parent ),
    m_includeHidden( false ),
    m_folder( NULL ),
    m_hasParent( false ),
    m_sortColumn( -1 ),
    m_sortOrder( Qt::AscendingOrder ),
    m_pendingRefresh( false ),
    m_pendingUpdate( false )
{
}

FolderItemModel::~FolderItemModel()
{
}

void FolderItemModel::setIncludeHidden( bool on )
{
    m_includeHidden = on;
}

void FolderItemModel::setFolder( ShellFolder* folder )
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    delete m_folder;
    m_folder = folder;

    ShellFolder::Flags flags = 0;
    if ( m_includeHidden )
        flags |= ShellFolder::IncludeHidden;

    m_items = m_folder->listItems( flags );
    qSort( m_items.begin(), m_items.end(), ShellItemLessThan( this ) );

    connect( m_folder, SIGNAL( itemChanged( const ItemChange& ) ), this, SLOT( itemChanged( const ItemChange& ) ) );
    connect( m_folder, SIGNAL( folderUpdated() ), this, SLOT( folderUpdated() ) );

    m_folder->startWatching();

    m_hasParent = m_folder->hasParent();

    m_extractQueue.clear();
    m_changes.clear();
    m_pendingRefresh = false;

    reset();

    QApplication::restoreOverrideCursor();
}

void FolderItemModel::refresh()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit layoutAboutToBeChanged();

    QModelIndexList oldIndexes = persistentIndexList();
    QStringList names = storeIndexes( oldIndexes );

    ShellFolder::Flags flags = 0;
    if ( m_includeHidden )
        flags |= ShellFolder::IncludeHidden;

    m_items = m_folder->listItems( flags );
    qSort( m_items.begin(), m_items.end(), ShellItemLessThan( this ) );

    QModelIndexList newIndexes = restoreIndexes( names, oldIndexes );
    changePersistentIndexList( oldIndexes, newIndexes );

    m_extractQueue.clear();
    m_changes.clear();
    m_pendingRefresh = false;

    emit layoutChanged();

    QApplication::restoreOverrideCursor();
}

void FolderItemModel::itemChanged( const ItemChange& change )
{
    if ( !m_pendingRefresh ) {
        m_changes.append( change );

        if ( !m_pendingUpdate ) {
            QTimer::singleShot( 100, this, SLOT( applyChanges() ) );
            m_pendingUpdate = true;
        }
    }
}

void FolderItemModel::folderUpdated()
{
    if ( !m_pendingRefresh ) {
        m_changes.clear();
        m_pendingRefresh = true;

        if ( !m_pendingUpdate ) {
            QTimer::singleShot( 100, this, SLOT( applyChanges() ) );
            m_pendingUpdate = true;
        }
    }
}

void FolderItemModel::applyChanges()
{
    if ( m_pendingRefresh )
        refresh();

    if ( !m_changes.isEmpty() ) {
        emit layoutAboutToBeChanged();

        QModelIndexList oldIndexes = persistentIndexList();
        QStringList names = storeIndexes( oldIndexes );

        for ( int i = 0; i < m_changes.count(); i++ ) {
            ItemChange& change = m_changes[ i ];
            bool found = false;

            for ( int j = 0; j < m_items.count(); j++ ) {
                ShellItem& item = m_items[ j ];

                if ( item.match( change.item1() ) ) {
                    switch ( change.type() ) {
                        case ItemChange::ItemAdded:
                        case ItemChange::ItemUpdated:
                            if ( item.isSelected() )
                                change.item1().setSelected( true );
                            item = change.item1();
                            break;

                        case ItemChange::ItemRemoved:
                            for ( int k = 0; k < names.count(); k++ ) {
                                QString& name = names[ k ];
                                if ( name == item.name() )
                                    name.clear();
                            }
                            m_items.removeAt( j );
                            break;

                        case ItemChange::ItemRenamed:
                            for ( int k = 0; k < names.count(); k++ ) {
                                QString& name = names[ k ];
                                if ( name == item.name() )
                                    name = change.item2().name();
                            }
                            if ( item.isSelected() )
                                change.item2().setSelected( true );
                            item = change.item2();
                            break;
                    }

                    found = true;
                    break;
                }
            }

            if ( !found && change.type() == ItemChange::ItemAdded ) {
                if ( m_includeHidden || !change.item1().attributes().testFlag( ShellItem::Hidden ) )
                    m_items.append( change.item1() );
            }
        }

        qSort( m_items.begin(), m_items.end(), ShellItemLessThan( this ) );

        QModelIndexList newIndexes = restoreIndexes( names, oldIndexes );
        changePersistentIndexList( oldIndexes, newIndexes );

        m_extractQueue.clear();
        m_changes.clear();

        emit layoutChanged();
    }

    m_pendingUpdate = false;
}

QStringList FolderItemModel::storeIndexes( const QModelIndexList& indexes )
{
    QStringList names;

    foreach ( QModelIndex index, indexes )
        names.append( itemAt( index ).name() );

    return names;
}

QModelIndexList FolderItemModel::restoreIndexes( const QStringList& names, const QModelIndexList& oldIndexes )
{
    QModelIndexList newIndexes;

    for ( int i = 0; i < names.count(); i++ ) {
        QModelIndex index;
        for ( int j = 0; j < m_items.count(); j++ ) {
            if ( m_items.at( j ).name() == names.at( i ) ) {
                index = rowToIndex( j, oldIndexes.at( i ).column() );
                break;
            }
        }
        if ( !index.isValid() && m_hasParent )
            index = createIndex( 0, oldIndexes.at( i ).column() );
        newIndexes.append( index );
    }

    return newIndexes;
}

void FolderItemModel::extractNextIcon()
{
    if ( m_extractQueue.isEmpty() )
        return;

    int row = m_extractQueue.takeFirst();

    m_folder->extractIcon( m_items[ row ] );

    QModelIndex index = rowToIndex( row, 0 );
    emit dataChanged( index, index );

    if ( !m_extractQueue.isEmpty() )
        QTimer::singleShot( 0, this, SLOT( extractNextIcon() ) );
}

int FolderItemModel::indexToRow( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return -1;

    int row = index.row() - ( m_hasParent ? 1 : 0 );
    if ( row < 0 || row >= m_items.count() )
        return -1;

    return row;
}

QModelIndex FolderItemModel::rowToIndex( int row, int column ) const
{
    return createIndex( row + ( m_hasParent ? 1 : 0 ), column );
}

ShellItem FolderItemModel::itemAt( const QModelIndex& index ) const
{
    int row = indexToRow( index );
    if ( row < 0 )
        return ShellItem();
    return m_items.at( row );
}

QModelIndex FolderItemModel::indexOf( const ShellItem& item, int column /*= 0*/ ) const
{
    for ( int i = 0; i < m_items.count(); i++ ) {
        if ( m_items.at( i ).match( item ) )
            return rowToIndex( i, column );
    }
    return QModelIndex();
}

bool FolderItemModel::isParentFolder( const QModelIndex& index ) const
{
    return m_hasParent && index.isValid() && index.row() == 0;
}

void FolderItemModel::setItemSelected( const QModelIndex& index, bool selected )
{
    int row = indexToRow( index );
    if ( row >= 0 ) {
        m_items[ row ].setSelected( selected );
        emit dataChanged( rowToIndex( row, 0 ), rowToIndex( row, 3 ) );
    }
}

bool FolderItemModel::isItemSelected( const QModelIndex& index ) const
{
    int row = indexToRow( index );
    if ( row < 0 )
        return false;
    return m_items.at( row ).isSelected();
}

void FolderItemModel::toggleItemSelected( const QModelIndex& index )
{
    int row = indexToRow( index );
    if ( row >= 0 ) {
        m_items[ row ].setSelected( !m_items.at( row ).isSelected() );
        emit dataChanged( rowToIndex( row, 0 ), rowToIndex( row, 3 ) );
    }
}

QList<ShellItem> FolderItemModel::selectedItems() const
{
    QList<ShellItem> result;

    foreach ( ShellItem item, m_items ) {
        if ( item.isSelected() )
            result.append( item );
    }

    return result;
}

void FolderItemModel::selectAll()
{
    for ( int i = 0; i < m_items.count(); i++ )
        m_items[ i ].setSelected( true );
    emit dataChanged( rowToIndex( 0, 0 ), rowToIndex( m_items.count() - 1, 3 ) );
}

void FolderItemModel::unselectAll()
{
    for ( int i = 0; i < m_items.count(); i++ )
        m_items[ i ].setSelected( false );
    emit dataChanged( rowToIndex( 0, 0 ), rowToIndex( m_items.count() - 1, 3 ) );
}

void FolderItemModel::invertSelection()
{
    for ( int i = 0; i < m_items.count(); i++ )
        m_items[ i ].setSelected( !m_items[ i ].isSelected() );
    emit dataChanged( rowToIndex( 0, 0 ), rowToIndex( m_items.count() - 1, 3 ) );
}

void FolderItemModel::calculateSize( const QModelIndex& index )
{
    int row = indexToRow( index );
    if ( row >= 0 ) {
        QApplication::setOverrideCursor( Qt::WaitCursor );

        if ( m_folder->calculateSize( m_items[ row ] ) ) {
            if ( m_sortColumn == Column_Size )
                resort();
            else
                emit dataChanged( rowToIndex( row, 0 ), rowToIndex( row, 3 ) );
        }

        QApplication::restoreOverrideCursor();
    }
}

qint64 FolderItemModel::totalItemsSize() const
{
    qint64 size = 0;

    foreach ( ShellItem item, m_items )
        size += item.size();

    return size;
}

qint64 FolderItemModel::selectedItemsSize() const
{
    qint64 size = 0;

    foreach ( ShellItem item, m_items ) {
        if ( item.isSelected() )
            size += item.size();
    }

    return size;
}

int FolderItemModel::totalItemsCount() const
{
    return m_items.count();
}

int FolderItemModel::selectedItemsCount() const
{
    int count = 0;

    foreach ( ShellItem item, m_items ) {
        if ( item.isSelected() )
            count++;
    }

    return count;
}

int FolderItemModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return 4;
    return 0;
}

QVariant FolderItemModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        switch ( section ) {
            case Column_Name:
                return tr( "Name" );
            case Column_Size:
                return tr( "Size" );
            case Column_LastModified:
                return tr( "Modified" );
            case Column_Attributes:
                return tr( "Attributes" );
        }
    }
    return QVariant();
}

int FolderItemModel::rowCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return m_items.count() + ( m_hasParent ? 1 : 0 );
    return 0;
}

QModelIndex FolderItemModel::index( int row, int column, const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !parent.isValid() )
        return createIndex( row, column );
    return QModelIndex();
}

QModelIndex FolderItemModel::parent( const QModelIndex& /*index*/ ) const
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

QVariant FolderItemModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    if ( role == Qt::DisplayRole || role == Qt::EditRole ) {
        if ( isParentFolder( index ) ) {
            if ( index.column() == Column_Name )
                return "..";
            if ( index.column() == Column_Size )
                return tr( "<DIR>" );
            return QVariant();
        }

        int row = indexToRow( index );
        if ( row >= 0 ) {
            const ShellItem& item = m_items.at( row );

            switch ( index.column() ) {
                case Column_Name:
                    return item.name();
                case Column_Size:
                    if ( item.attributes().testFlag( ShellItem::Directory ) ) {
                        if ( item.attributes().testFlag( ShellItem::ReparsePoint ) )
                            return tr( "<LNK>" );
                        if ( !item.state().testFlag( ShellItem::HasCalculatedSize ) )
                            return tr( "<DIR>" );
                    } 
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
        if ( isParentFolder( index ) )
            return IconLoader::pixmap( "arrow-up" );

        int row = indexToRow( index );
        if ( row >= 0 ) {
            if ( !( m_items.at( row ).state().testFlag( ShellItem::HasExtractedIcon ) ) && !m_extractQueue.contains( row ) ) {
                FolderItemModel* that = const_cast<FolderItemModel*>( this );
                if ( m_extractQueue.isEmpty() )
                    QTimer::singleShot( 0, that, SLOT( extractNextIcon() ) );
                that->m_extractQueue.append( row );
            }

            return m_items.at( row ).icon();
        }
    }

    if ( role == Qt::ToolTipRole && index.column() == Column_Name ) {
        int row = indexToRow( index );
        if ( row >= 0 ) {
            if ( !m_items.at( row ).attributes().testFlag( ShellItem::Directory ) )
                return m_folder->toolTip( m_items.at( row ) );
        }
    }

    if ( role == Qt::TextAlignmentRole && index.column() == Column_Size ) {
        int row = indexToRow( index );
        if ( row >= 0 ) {
            const ShellItem& item = m_items.at( row );
            if ( !item.attributes().testFlag( ShellItem::Directory ) || item.state().testFlag( ShellItem::HasCalculatedSize ) )
                return (int)( Qt::AlignRight | Qt::AlignVCenter );
        }
    }

    if ( role == Qt::ForegroundRole ) {
        int row = indexToRow( index );
        if ( row >= 0 ) {
            const ShellItem& item = m_items.at( row );

            if ( item.isSelected() )
                return QColor( 255, 0, 0 );

            if ( item.attributes().testFlag( ShellItem::Hidden ) )
                return QColor( 128, 128, 128 );

            if ( item.attributes().testFlag( ShellItem::Compressed ) )
                return QColor( 0, 0, 128 );
        }
    }

    return QVariant();
}

bool FolderItemModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
    if ( role == Qt::EditRole ) {
        int row = indexToRow( index );
        if ( row >= 0 && index.column() == 0 ) {
            if ( m_folder->setItemName( m_items[ row ], value.toString() ) ) {
                resort();
                return true;
            }
        }
    }

    return false;
}

Qt::ItemFlags FolderItemModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    Qt::ItemFlags result = Qt::ItemIsEnabled;

    int row = indexToRow( index );
    if ( row >= 0 && index.column() == 0 ) {
        if ( m_items.at( row ).attributes().testFlag( ShellItem::CanRename ) )
            result |= Qt::ItemIsEditable;
    }

    return result;
}

void FolderItemModel::sort( int column, Qt::SortOrder order /*= Qt::AscendingOrder*/ )
{
    if ( m_sortColumn == column && m_sortOrder == order )
        return;

    m_sortColumn = column;
    m_sortOrder = order;

    resort();
}

void FolderItemModel::resort()
{
    emit layoutAboutToBeChanged();

    QModelIndexList oldIndexes = persistentIndexList();
    QStringList names = storeIndexes( oldIndexes );

    qSort( m_items.begin(), m_items.end(), ShellItemLessThan( this ) );

    QModelIndexList newIndexes = restoreIndexes( names, oldIndexes );
    changePersistentIndexList( oldIndexes, newIndexes );

    m_extractQueue.clear();

    emit layoutChanged();
}

bool FolderItemModel::shellItemLessThan( const ShellItem& item1, const ShellItem& item2 )
{
    bool dir1 = ( item1.attributes().testFlag( ShellItem::Directory ) );
    bool dir2 = ( item2.attributes().testFlag( ShellItem::Directory ) );

    if ( dir1 && !dir2 )
        return true;
    if ( !dir1 && dir2 )
        return false;

    int result = 0;

    switch ( m_sortColumn ) {
        case Column_Size:
            if ( item1.size() < item2.size() )
                result = -1;
            else if ( item2.size() < item1.size() )
                result = 1;
            break;
        case Column_LastModified:
            if ( item1.lastModified() < item2.lastModified() )
                result = -1;
            else if ( item2.lastModified() < item1.lastModified() )
                result = 1;
            break;
        case Column_Attributes:
            result = QString::localeAwareCompare( formatAttributes( item1.attributes() ), formatAttributes( item2.attributes() ) );
            break;
    }

    if ( result == 0 )
        result = QString::localeAwareCompare( item1.name(), item2.name() );

    if ( m_sortOrder == Qt::AscendingOrder )
        return result < 0;
    else
        return result > 0;
}
