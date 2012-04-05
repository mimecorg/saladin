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

#include "separatorcombobox.h"

#include <QItemDelegate>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QListView>
#include <QPainter>
#include <QKeyEvent>

class SeparatorItemDelegate : public QAbstractItemDelegate
{
public:
    static const int ItemTypeRole = Qt::UserRole + 1;

    enum ItemType {
        NormalItem,
        SeparatorItem,
        ParentItem,
        ChildItem 
    };

public:
    SeparatorItemDelegate( QObject* parent, QAbstractItemDelegate* delegate ) : QAbstractItemDelegate( parent ),
        m_delegate( delegate )
    {
    }

    ~SeparatorItemDelegate()
    {
    }

public: // overrides
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        int type = index.data( ItemTypeRole ).toInt();
        switch ( type ) {
            case SeparatorItem: {
                QStyleOptionViewItem noFocus = option;
                noFocus.state &= ~QStyle::State_HasFocus;

                m_delegate->paint( painter, noFocus, index );

                int y = ( option.rect.top() + option.rect.bottom() ) / 2;

                painter->setPen(  option.palette.color( QPalette::Active, QPalette::Dark ) );
                painter->drawLine( option.rect.left(), y, option.rect.right(), y );
                break;
            }
            case ParentItem: {
                QStyleOptionViewItem parentOption = option;
                parentOption.state |= QStyle::State_Enabled;
                m_delegate->paint( painter, parentOption, index );
                break;
            }
            case ChildItem: {
                QStyleOptionViewItem childOption = option;
                int indent = option.fontMetrics.width( "    " );
                childOption.rect.adjust( indent, 0, 0, 0 );
                childOption.textElideMode = Qt::ElideNone;
                m_delegate->paint( painter, childOption, index );
                break;
            }
            default:
                m_delegate->paint( painter, option, index );
                break;
        }
    }

    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        QSize size = m_delegate->sizeHint( option, index );

        int type = index.data( ItemTypeRole ).toInt();
        if ( type == SeparatorItem )
            size.setHeight( 5 );
        return size;
    }

private:
    QAbstractItemDelegate* m_delegate;
};

SeparatorComboBox::SeparatorComboBox( QWidget* parent ) : QComboBox( parent )
{
    QAbstractItemDelegate* delegate = view()->itemDelegate();
    view()->setItemDelegate( NULL );

    setItemDelegate( new SeparatorItemDelegate( this, delegate ) );
}

SeparatorComboBox::~SeparatorComboBox()
{
}

void SeparatorComboBox::addSeparator()
{
    QStandardItem* item = new QStandardItem( QString::null );
    item->setFlags( item->flags() & ~( Qt::ItemIsEnabled | Qt::ItemIsSelectable ) );
    item->setData( SeparatorItemDelegate::SeparatorItem, SeparatorItemDelegate::ItemTypeRole );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow( item );
}

void SeparatorComboBox::addParentItem( const QString& text )
{
    QStandardItem* item = new QStandardItem( text );
    item->setFlags( item->flags() & ~( Qt::ItemIsEnabled | Qt::ItemIsSelectable ) );
    item->setData( SeparatorItemDelegate::ParentItem, SeparatorItemDelegate::ItemTypeRole );

    QFont font = item->font();
    font.setBold( true );
    item->setFont( font );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow( item );
}

void SeparatorComboBox::addChildItem( const QString& text, const QVariant& data /*= QVariant()*/ )
{
    QStandardItem* item = new QStandardItem( text + QLatin1String( "    " ) );
    item->setData( data, Qt::UserRole );
    item->setData( SeparatorItemDelegate::ChildItem, SeparatorItemDelegate::ItemTypeRole );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow( item );
}
