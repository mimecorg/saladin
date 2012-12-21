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

#include "shell/shellfolder.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "xmlui/gradientwidget.h"

SearchDialog::SearchDialog( ShellFolder* folder, QWidget* parent ) : QDialog( parent ),
    m_folder( folder )
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
    gridLayout->addWidget( m_locationEdit, 0, 1 );

    locationLabel->setBuddy( m_locationEdit );

    QLabel* patternLabel = new QLabel( tr( "&Pattern:" ), this );
    gridLayout->addWidget( patternLabel, 1, 0 );

    m_patternEdit = new QLineEdit( this );
    gridLayout->addWidget( m_patternEdit, 1, 1 );

    patternLabel->setBuddy( m_patternEdit );

    mainLayout->addSpacing( 5 );

    m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
    mainLayout->addWidget( m_buttonBox );

    m_buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&Search" ) );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );

    connect( m_buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    QGroupBox* resultsBox = new QGroupBox( tr( "Search Results" ), this );
    mainLayout->addWidget( resultsBox );

    QHBoxLayout* resultsLayout = new QHBoxLayout( resultsBox );

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

    m_model = new SearchItemModel( this );

    connect( m_model, SIGNAL( searchCompleted() ), this, SLOT( searchCompleted() ) );

    m_proxyModel = new SearchProxyModel( this );
    m_proxyModel->setSortLocaleAware( true );
    m_proxyModel->setDynamicSortFilter( true );

    m_view->setModel( m_proxyModel );
    m_proxyModel->setSourceModel( m_model );

    m_view->setSortingEnabled( true );
    m_view->sortByColumn( 0, Qt::AscendingOrder );

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

void SearchDialog::accept()
{
    QString pattern = m_patternEdit->text();
    if ( pattern.isEmpty() ) {
        QMessageBox::warning( this, tr( "Invalid value" ), tr( "Pattern cannot be empty." ) );
        return;
    }

    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "Pattern", pattern );

    m_buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Abort" ) );

    m_model->startSearch( m_folder, pattern );
}

void SearchDialog::reject()
{
    if ( m_model->isSearching() ) {
        m_model->abortSearch();
        searchCompleted();
    } else {
        QDialog::reject();
    }
}

void SearchDialog::searchCompleted()
{
    m_buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    m_buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Close" ) );
}
