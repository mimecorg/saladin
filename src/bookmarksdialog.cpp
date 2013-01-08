/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2013 Michał Męciński
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

#include "bookmarksdialog.h"
#include "operationdialog.h"
#include "application.h"

#include "utils/iconloader.h"
#include "xmlui/gradientwidget.h"
#include "xmlui/builder.h"
#include "xmlui/toolstrip.h"

BookmarksDialog::BookmarksDialog( QWidget* parent ) : QDialog( parent )
{
    setWindowTitle( tr( "Edit Bookmarks" ) );

    QAction* action;

    action = new QAction( IconLoader::icon( "rename" ), tr( "Rename Bookmark..." ), this );
    action->setShortcut( Qt::Key_F2 );
    connect( action, SIGNAL( triggered() ), this, SLOT( editRename() ) );
    setAction( "editRename", action );

    action = new QAction( IconLoader::icon( "delete" ), tr( "Delete Bookmark" ), this );
    action->setShortcut( QKeySequence::Delete );
    connect( action, SIGNAL( triggered() ), this, SLOT( editDelete() ) );
    setAction( "editDelete", action );

    loadXmlUiFile( ":/resources/bookmarksdialog.xml" );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->addClient( this );

    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    XmlUi::GradientWidget* promptWidget = new XmlUi::GradientWidget( this );
    topLayout->addWidget( promptWidget );

    QHBoxLayout* promptLayout = new QHBoxLayout( promptWidget );
    promptLayout->setSpacing( 10 );

    QLabel* promptPixmap = new QLabel( promptWidget );
    promptPixmap->setPixmap( IconLoader::pixmap( "edit", 22 ) );
    promptLayout->addWidget( promptPixmap );

    QLabel* promptLabel = new QLabel( promptWidget );
    promptLabel->setWordWrap( true );
    promptLabel->setText( tr( "Edit your list of bookmarks:" ) );
    promptLabel->setMinimumWidth( 350 );
    promptLabel->setFixedHeight( promptLabel->heightForWidth( 350 ) );
    promptLayout->addWidget( promptLabel, 1 );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    topLayout->addWidget( separator );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin( 9 );
    mainLayout->setSpacing( 4 );
    topLayout->addLayout( mainLayout );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    builder->registerToolStrip( "stripBookmarks", strip );
    mainLayout->addWidget( strip );

    m_list = new QTreeWidget( this );
    m_list->setSelectionMode( QAbstractItemView::SingleSelection );
    m_list->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_list->setRootIsDecorated( false );
    m_list->setAllColumnsShowFocus( true );
    m_list->setUniformRowHeights( true );
    m_list->header()->setStretchLastSection( false );
    m_list->setContextMenuPolicy( Qt::CustomContextMenu );
    mainLayout->addWidget( m_list, 1 );

    QTreeWidgetItem* header = new QTreeWidgetItem();
    header->setText( 0, tr( "Name" ) );
    header->setText( 1, tr( "Path" ) );
    m_list->setHeaderItem( header );

    m_list->setColumnWidth( 0, 240 );
    m_list->setColumnWidth( 1, 400 );

    connect( m_list->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
        this, SLOT( updateActions() ) );
    connect( m_list, SIGNAL( doubleClicked( const QModelIndex& ) ),
        this, SLOT( doubleClicked( const QModelIndex& ) ) );
    connect( m_list, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( listContextMenu( const QPoint& ) ) );

    mainLayout->addSpacing( 7 );

    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
    mainLayout->addWidget( buttonBox );

    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );
    buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );

    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    resize( 700, 500 );

    m_bookmarks = application->bookmarks();
    qSort( m_bookmarks );

    populateList();

    updateActions();
}

BookmarksDialog::~BookmarksDialog()
{
}

void BookmarksDialog::accept()
{
    application->setBookmarks( m_bookmarks );

    QDialog::accept();
}

void BookmarksDialog::editRename()
{
    QModelIndexList selection = m_list->selectionModel()->selectedRows();

    if ( !selection.isEmpty() ) {
        int index = selection.first().row();

        OperationDialog dialog( OperationDialog::WithName | OperationDialog::CanEditName, this );

        dialog.setWindowTitle( tr( "Rename Bookmark" ) );
        dialog.setPromptPixmap( IconLoader::pixmap( "rename", 22 ) );
        dialog.setPrompt( tr( "Enter the new name of bookmark <b>%1</b>:" ).arg( m_bookmarks.at( index ).name() ) );

        dialog.setName( m_bookmarks.at( index ).name() );

        if ( dialog.exec() != QDialog::Accepted )
            return;

        m_bookmarks[ index ].setName( dialog.name() );
        qSort( m_bookmarks );

        populateList();
    }
}

void BookmarksDialog::editDelete()
{
    QModelIndexList selection = m_list->selectionModel()->selectedRows();

    if ( !selection.isEmpty() ) {
        int index = selection.first().row();

        OperationDialog dialog( 0, this );

        dialog.setWindowTitle( tr( "Delete Bookmark" ) );
        dialog.setPromptPixmap( IconLoader::pixmap( "delete", 22 ) );
        dialog.setPrompt( tr( "Are you sure you want to delete bookmark <b>%1</b>?" ).arg( m_bookmarks.at( index ).name() ) );

        dialog.setName( m_bookmarks.at( index ).name() );

        if ( dialog.exec() != QDialog::Accepted )
            return;

        m_bookmarks.removeAt( index );

        delete m_list->topLevelItem( index );
    }
}

void BookmarksDialog::populateList()
{
    m_list->clear();

    foreach ( Bookmark bookmark, m_bookmarks ) {
        QTreeWidgetItem* item = new QTreeWidgetItem( m_list );
        item->setText( 0, bookmark.name() );
        item->setText( 1, bookmark.path() );
    }
}

void BookmarksDialog::updateActions()
{
    QModelIndexList selection = m_list->selectionModel()->selectedRows();

    action( "editRename" )->setEnabled( !selection.isEmpty() );
    action( "editDelete" )->setEnabled( !selection.isEmpty() );
}

void BookmarksDialog::doubleClicked( const QModelIndex& index )
{
    if ( index.isValid() )
        editRename();
}

void BookmarksDialog::listContextMenu( const QPoint& pos )
{
    QModelIndex index = m_list->indexAt( pos );

    if ( index.isValid() ) {
        m_list->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
        m_list->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

        QMenu* menu = builder()->contextMenu( "menuBookmark" );
        if ( menu )
            menu->exec( m_list->viewport()->mapToGlobal( pos ) );
    }
}
