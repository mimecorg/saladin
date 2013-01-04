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

#include "searchdialog.h"
#include "searchitemmodel.h"
#include "folderitemview.h"
#include "application.h"
#include "mainwindow.h"

#include "shell/shellfolder.h"
#include "utils/elidedlabel.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "viewer/viewmanager.h"
#include "xmlui/gradientwidget.h"
#include "xmlui/toolstrip.h"

SearchDialog::SearchDialog( ShellFolder* folder, QWidget* parent ) : QDialog( parent ),
    m_folder( folder ),
    m_textCheckBox( NULL ),
    m_textEdit( NULL ),
    m_caseCheckBox( NULL )
{
    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    XmlUi::GradientWidget* promptWidget = new XmlUi::GradientWidget( this );
    topLayout->addWidget( promptWidget );

    QHBoxLayout* promptLayout = new QHBoxLayout( promptWidget );
    promptLayout->setSpacing( 10 );

    QLabel* promptPixmap = new QLabel( promptWidget );
    promptLayout->addWidget( promptPixmap );

    QLabel* promptLabel = new QLabel( promptWidget );
    promptLabel->setWordWrap( true );
    promptLayout->addWidget( promptLabel, 1 );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    topLayout->addWidget( separator );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin( 9 );
    mainLayout->setSpacing( 6 );
    topLayout->addLayout( mainLayout );

    QGridLayout* gridLayout = new QGridLayout();
    mainLayout->addLayout( gridLayout );

    QLabel* locationLabel = new QLabel( tr( "Directory:" ), this );
    gridLayout->addWidget( locationLabel, 0, 0 );

    m_locationEdit = new QLineEdit( this );
    m_locationEdit->setReadOnly( true );
    gridLayout->addWidget( m_locationEdit, 0, 1, 1, 2 );

    locationLabel->setBuddy( m_locationEdit );

    QLabel* patternLabel = new QLabel( tr( "&Pattern:" ), this );
    gridLayout->addWidget( patternLabel, 1, 0 );

    m_patternEdit = new QLineEdit( this );
    gridLayout->addWidget( m_patternEdit, 1, 1, 1, 2 );

    patternLabel->setBuddy( m_patternEdit );

    if ( folder->attributes().testFlag( ShellItem::FileSystem ) ) {
        m_textCheckBox = new QCheckBox( tr( "&Text:" ), this );
        gridLayout->addWidget( m_textCheckBox, 2, 0 );

        m_textEdit = new QLineEdit( this );
        m_textEdit->setEnabled( false );
        gridLayout->addWidget( m_textEdit, 2, 1 );

        m_caseCheckBox = new QCheckBox( tr( "&Match case" ), this );
        m_caseCheckBox->setEnabled( false );
        gridLayout->addWidget( m_caseCheckBox, 2, 2 );

        connect( m_textCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( textToggled( bool ) ) );
    }

    gridLayout->setColumnStretch( 1, 1 );

    mainLayout->addSpacing( 5 );

    m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
    mainLayout->addWidget( m_buttonBox );

    m_buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&Search" ) );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );

    connect( m_buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    QGroupBox* resultsBox = new QGroupBox( tr( "Search Results" ), this );
    mainLayout->addWidget( resultsBox );

    QVBoxLayout* resultsLayout = new QVBoxLayout( resultsBox );

    QAction* viewAction = new QAction( IconLoader::icon( "view" ), tr( "View" ), this );
    viewAction->setShortcut( QKeySequence( Qt::Key_F3 ) );
    connect( viewAction, SIGNAL( triggered() ), this, SLOT( viewCurrent() ) );

    QAction* editAction = new QAction( IconLoader::icon( "edit" ), tr( "Edit" ), this );
    editAction->setShortcut( QKeySequence( Qt::Key_F4 ) );
    connect( editAction, SIGNAL( triggered() ), this, SLOT( editCurrent() ) );

    QAction* gotoAction = new QAction( IconLoader::icon( "goto" ), tr( "Go To File" ), this );
    gotoAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
    connect( gotoAction, SIGNAL( triggered() ), this, SLOT( gotoFile() ) );

    QAction* copyAction = new QAction( IconLoader::icon( "copy-names" ), tr( "Copy File Names" ), this );
    copyAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
    connect( copyAction, SIGNAL( triggered() ), this, SLOT( copyNames() ) );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( resultsBox );
    resultsLayout->addWidget( strip );

    strip->addToolAction( viewAction );
    strip->addToolAction( editAction );
    strip->addToolAction( gotoAction );
    strip->addAuxiliaryAction( copyAction );

    m_view = new FolderItemView( resultsBox );
    m_view->setSelectionMode( QAbstractItemView::NoSelection );
    m_view->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_view->setRootIsDecorated( false );
    m_view->setAlternatingRowColors( true );
    m_view->setAllColumnsShowFocus( true );
    m_view->setUniformRowHeights( true );
    m_view->header()->setStretchLastSection( false );
    m_view->setContextMenuPolicy( Qt::CustomContextMenu );
    resultsLayout->addWidget( m_view );

    connect( m_view, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( contextMenuRequested( const QPoint& ) ) );

    m_model = new SearchItemModel( this );

    connect( m_model, SIGNAL( folderEntered( const QString& ) ), this, SLOT( folderEntered( const QString& ) ) );
    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateResults() ) );
    connect( m_model, SIGNAL( rowsInserted ( const QModelIndex&, int, int ) ), this, SLOT( updateResults() ) );
    connect( m_model, SIGNAL( searchCompleted() ), this, SLOT( searchCompleted() ) );

    m_proxyModel = new SearchProxyModel( this );
    m_proxyModel->setSortLocaleAware( true );
    m_proxyModel->setDynamicSortFilter( true );

    m_view->setModel( m_proxyModel );
    m_proxyModel->setSourceModel( m_model );

    m_view->setSortingEnabled( true );
    m_view->sortByColumn( 0, Qt::AscendingOrder );

    QStatusBar* status = new QStatusBar( this );
    topLayout->addWidget( status );

    m_searchStatus = new ElidedLabel( status );
    status->addWidget( m_searchStatus, 2 );

    m_itemsStatus = new ElidedLabel( status );
    status->addWidget( m_itemsStatus, 1 );

    setWindowTitle( tr( "Search" ) );
    promptPixmap->setPixmap( IconLoader::pixmap( "find", 22 ) );
    promptLabel->setText( tr( "Search for files:" ) );

    promptLabel->setMinimumWidth( 350 );
    promptLabel->setFixedHeight( promptLabel->heightForWidth( 350 ) );

    m_locationEdit->setText( folder->path() );

    LocalSettings* settings = application->applicationSettings();

    if ( settings->contains( "SearchHeaderState" ) ) {
        m_view->header()->restoreState( settings->value( "SearchHeaderState" ).toByteArray() );
    } else {
        m_view->setColumnWidth( 0, 410 );
        m_view->setColumnWidth( 1, 90 );
        m_view->setColumnWidth( 2, 110 );
        m_view->setColumnWidth( 3, 70 );
    }

    m_patternEdit->setText( settings->value( "Pattern" ).toString() );

    if ( m_textEdit ) {
        m_textEdit->setText( settings->value( "FindText" ).toString() );
        m_caseCheckBox->setChecked( ( (QTextDocument::FindFlags)settings->value( "FindFlags" ).toInt() & QTextDocument::FindCaseSensitively ) != 0 );
    }

    m_patternEdit->setFocus();
    m_patternEdit->selectAll();

    resize( settings->value( "SearchDialogSize", QSize( 750, 550 ) ).toSize() );
}

SearchDialog::~SearchDialog()
{
    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "SearchHeaderState", m_view->header()->saveState() );
    settings->setValue( "SearchDialogSize", size() );
}

void SearchDialog::textToggled( bool on )
{
    m_textEdit->setEnabled( on );
    m_caseCheckBox->setEnabled( on );

    if ( on ) {
        m_textEdit->selectAll();
        m_textEdit->setFocus();
    }
}

void SearchDialog::accept()
{
    QString pattern = m_patternEdit->text();
    if ( pattern.isEmpty() ) {
        QMessageBox::warning( this, tr( "Invalid value" ), tr( "Pattern cannot be empty." ) );
        return;
    }

    QString text;
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;

    if ( m_textCheckBox && m_textCheckBox->isChecked() ) {
        text = m_textEdit->text();
        if ( text.isEmpty() ) {
            QMessageBox::warning( this, tr( "Invalid value" ), tr( "Text cannot be empty." ) );
            return;
        }
        cs = m_caseCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    }

    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "Pattern", pattern );

    if ( !text.isEmpty() ) {
        settings->setValue( "FindText", text );
        settings->setValue( "FindFlags", (int)( cs == Qt::CaseSensitive ? QTextDocument::FindCaseSensitively : 0 ) );
    }

    m_buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Abort" ) );

    m_searchStatus->setText( tr( "Searching..." ) );

    m_model->startSearch( m_folder, pattern, text, cs );

    m_view->setFocus();
}

void SearchDialog::reject()
{
    if ( !m_model->isSearching() ) {
        QDialog::reject();
        return;
    }

    m_model->abortSearch();

    m_buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Close" ) );

    m_searchStatus->setText( tr( "Search aborted." ) );
}

void SearchDialog::updateResults()
{
    m_itemsStatus->setText( tr( "%1 items found" ).arg( m_model->rowCount() ) );
}

void SearchDialog::folderEntered( const QString& path )
{
    m_searchStatus->setText( tr( "Searching %1..." ).arg( path ) );
}

void SearchDialog::searchCompleted()
{
    m_buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Close" ) );

    m_searchStatus->setText( tr( "Search completed." ) );
}

void SearchDialog::contextMenuRequested( const QPoint& pos )
{
    if ( !m_view->viewport()->visibleRegion().contains( pos ) )
        return;

    QModelIndex index = m_proxyModel->mapToSource( m_view->indexAt( pos ) );

    if ( index.isValid() ) {
        QList<ShellItem> items;
        items.append( m_model->itemAt( index ) );

        m_view->setCurrentIndex( index );

        ShellSelection selection( m_model->folderAt( index ), items, this );
        selection.showContextMenu( m_view->viewport()->mapToGlobal( pos ), 0 );
    }
}

void SearchDialog::viewCurrent()
{
    QModelIndex index = m_proxyModel->mapToSource( m_view->currentIndex() );

    ShellFolder* folder = m_model->folderAt( index );
    ShellItem item = m_model->itemAt( index );

    if ( !item.isValid() || !item.attributes().testFlag( ShellItem::FileSystem ) )
        return;

    LocalSettings* settings = application->applicationSettings();

    if ( settings->value( "InternalViewer" ).toBool() )
        mainWindow->viewManager()->openView( folder->itemPidl( item ) );
    else
        mainWindow->startTool( MainWindow::ViewerTool, folder, item );
}

void SearchDialog::editCurrent()
{
    QModelIndex index = m_proxyModel->mapToSource( m_view->currentIndex() );

    ShellFolder* folder = m_model->folderAt( index );
    ShellItem item = m_model->itemAt( index );

    mainWindow->startTool( MainWindow::EditorTool, folder, item );
}

void SearchDialog::gotoFile()
{
    QModelIndex index = m_proxyModel->mapToSource( m_view->currentIndex() );

    ShellFolder* folder = m_model->folderAt( index );
    ShellItem item = m_model->itemAt( index );

    mainWindow->gotoFile( folder->pidl(), item );

    QDialog::accept();
}

void SearchDialog::copyNames()
{
    QStringList names;

    for ( int i = 0; i < m_model->rowCount(); i++ ) {
        QModelIndex index = m_proxyModel->mapToSource( m_proxyModel->index( i, 0 ) );
        names.append( m_model->pathAt( index ) + m_model->itemAt( index ).name() );
    }

    QString text;
    if ( names.count() > 1 )
        text = names.join( "\r\n" ) + "\r\n";
    else
        text = names.first();

    QApplication::clipboard()->setText( text );
}
