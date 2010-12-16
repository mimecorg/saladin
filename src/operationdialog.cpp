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

#include "operationdialog.h"
#include "multirenamewidget.h"

#include "utils/iconloader.h"
#include "xmlui/gradientwidget.h"

OperationDialog::OperationDialog( Flags flags, QWidget* parent ) : QDialog( parent ),
    m_nameEdit( NULL ),
    m_patternEdit( NULL ),
    m_locationEdit( NULL ),
    m_sourceEdit( NULL ),
    m_targetEdit( NULL ),
    m_checkBox( NULL ),
    m_multiRename( NULL ),
    m_renameButton( NULL )
{
    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    XmlUi::GradientWidget* promptWidget = new XmlUi::GradientWidget( this );
    topLayout->addWidget( promptWidget );

    QHBoxLayout* promptLayout = new QHBoxLayout( promptWidget );
    promptLayout->setSpacing( 10 );

    m_promptPixmap = new QLabel( promptWidget );
    promptLayout->addWidget( m_promptPixmap );

    m_promptLabel = new QLabel( promptWidget );
    m_promptLabel->setWordWrap( true );
    promptLayout->addWidget( m_promptLabel, 1 );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    topLayout->addWidget( separator );

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setMargin( 9 );
    m_mainLayout->setSpacing( 6 );
    topLayout->addLayout( m_mainLayout );

    QGridLayout* gridLayout = new QGridLayout();
    m_mainLayout->addLayout( gridLayout );

    int row = 0;

    if ( flags.testFlag( WithName ) ) {
        QLabel* label = new QLabel( tr( "&Name:" ), this );
        gridLayout->addWidget( label, row, 0 );

        m_nameEdit = new QLineEdit( this );
        if ( !flags.testFlag( CanEditName ) )
            m_nameEdit->setReadOnly( true );
        gridLayout->addWidget( m_nameEdit, row++, 1 );

        label->setBuddy( m_nameEdit );
    }

    if ( flags.testFlag( WithPattern ) ) {
        QLabel* label = new QLabel( tr( "&Pattern:" ), this );
        gridLayout->addWidget( label, row, 0 );

        m_patternEdit = new QLineEdit( this );
        gridLayout->addWidget( m_patternEdit, row++, 1 );

        label->setBuddy( m_patternEdit );
    }

    if ( flags.testFlag( WithLocation ) ) {
        QLabel* label = new QLabel( tr( "Location:" ), this );
        gridLayout->addWidget( label, row, 0 );

        m_locationEdit = new QLineEdit( this );
        m_locationEdit->setReadOnly( true );
        gridLayout->addWidget( m_locationEdit, row++, 1 );

        label->setBuddy( m_locationEdit );
    }

    if ( flags.testFlag( WithSource ) ) {
        QLabel* label = new QLabel( tr( "Source:" ), this );
        gridLayout->addWidget( label, row, 0 );

        m_sourceEdit = new QLineEdit( this );
        m_sourceEdit->setReadOnly( true );
        gridLayout->addWidget( m_sourceEdit, row++, 1 );

        label->setBuddy( m_sourceEdit );
    }

    if ( flags.testFlag( WithTarget ) ) {
        QLabel* label = new QLabel( tr( "Target:" ), this );
        gridLayout->addWidget( label, row, 0 );

        m_targetEdit = new QLineEdit( this );
        m_targetEdit->setReadOnly( true );
        gridLayout->addWidget( m_targetEdit, row++, 1 );

        label->setBuddy( m_targetEdit );
    }

    if ( flags.testFlag( WithCheckBox ) ) {
        m_checkBox = new QCheckBox( this );
        gridLayout->addWidget( m_checkBox, row++, 1 );
    }

    if ( flags.testFlag( WithMultiRename ) )
        rename();

    if ( row > 0 )
        m_mainLayout->addSpacing( 5 );

    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
    m_mainLayout->addWidget( buttonBox );

    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );
    buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );

    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    if ( flags.testFlag( CanRename ) && !m_multiRename ) {
        m_renameButton = new QPushButton( tr( "&Rename..." ), buttonBox );
        m_renameButton->setIcon( IconLoader::icon( "arrow-down" ) );
        m_renameButton->setIconSize( QSize( 16, 16 ) );
        buttonBox->addButton( m_renameButton, QDialogButtonBox::ResetRole );

        connect( m_renameButton, SIGNAL( clicked() ), this, SLOT( rename() ) );
    }
}

OperationDialog::~OperationDialog()
{
}

void OperationDialog::setPromptPixmap( const QPixmap& pixmap )
{
    m_promptPixmap->setPixmap( pixmap );
}

void OperationDialog::setPrompt( const QString& text )
{
    m_promptLabel->setText( text );

    m_promptLabel->setMinimumWidth( 350 );
    m_promptLabel->setFixedHeight( m_promptLabel->heightForWidth( 350 ) );

    if ( !m_multiRename )
        setMaximumHeight( sizeHint().height() );
}

void OperationDialog::setName( const QString& text )
{
    if ( m_nameEdit ) {
        m_nameEdit->setText( text );
        if ( !m_nameEdit->isReadOnly() ) {
            int pos = text.lastIndexOf( '.' );
            if ( pos <= 0 || text.contains( QRegExp( "[:\\\\/]" ) ) )
                pos = text.length();
            m_nameEdit->setSelection( 0, pos );
        }
    }
}

void OperationDialog::setPattern( const QString& text )
{
    if ( m_patternEdit ) {
        m_patternEdit->setText( text );
        m_patternEdit->selectAll();
    }
}

void OperationDialog::setLocation( const QString& text )
{
    if ( m_locationEdit )
        m_locationEdit->setText( text );
}

void OperationDialog::setSource( const QString& text )
{
    if ( m_sourceEdit )
        m_sourceEdit->setText( text );
}

void OperationDialog::setTarget( const QString& text )
{
    if ( m_targetEdit )
        m_targetEdit->setText( text );
}

QString OperationDialog::name() const
{
    return m_nameEdit ? m_nameEdit->text() : QString();
}

QString OperationDialog::pattern() const
{
    return m_patternEdit ? m_patternEdit->text() : QString();
}

void OperationDialog::setCheckBoxText( const QString& text )
{
    if ( m_checkBox )
        m_checkBox->setText( text );
}

void OperationDialog::setCheckBoxChecked( bool checked )
{
    if ( m_checkBox )
        m_checkBox->setChecked( checked );
}

bool OperationDialog::checkBoxChecked() const
{
    return m_checkBox ? m_checkBox->isChecked() : false;
}

void OperationDialog::setNames( const QStringList& names )
{
    m_names = names;

    if ( m_multiRename )
        m_multiRename->setInputNames( names );
}

void OperationDialog::accept()
{
    if ( m_nameEdit && !m_nameEdit->isReadOnly() ) {
        QString name = m_nameEdit->text();
        if ( name.isEmpty() ) {
            QMessageBox::warning( this, tr( "Invalid value" ), tr( "Name cannot be empty." ) );
            return;
        }
    }

    if ( m_patternEdit ) {
        QString pattern = m_patternEdit->text();
        if ( pattern.isEmpty() ) {
            QMessageBox::warning( this, tr( "Invalid value" ), tr( "Pattern cannot be empty." ) );
            return;
        }
    }

    if ( m_multiRename ) {
        m_names = m_multiRename->outputNames();
        foreach ( QString name, m_names ) {
            if ( name.isEmpty() ) {
                QMessageBox::warning( this, tr( "Invalid value" ), tr( "Name cannot be empty." ) );
                return;
            }
        }
    }

    QDialog::accept();
}

void OperationDialog::rename()
{
    if ( !m_multiRename ) {
        setMaximumHeight( QWIDGETSIZE_MAX );

        QFrame* separator = new QFrame( this );
        separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        m_mainLayout->insertWidget( 1, separator );

        m_multiRename = new MultiRenameWidget( this );
        m_mainLayout->insertWidget( 2, m_multiRename, 1 );

        m_multiRename->setInputNames( m_names );

        if ( m_renameButton ) {
            m_renameButton->deleteLater();
            m_renameButton = NULL;
        }

        expandTo( QSize( 700, 500 ) );

        m_multiRename->setFocus( Qt::TabFocusReason );
    }
}

void OperationDialog::expandTo( const QSize& newSize )
{
    if ( isVisible() ) {
        QRect rect( QPoint( 0, 0 ), newSize + frameGeometry().size() - size() );
        rect.moveCenter( frameGeometry().center() );

        QRect screen = QApplication::desktop()->availableGeometry( this );

        if ( rect.bottom() > screen.bottom() )
            rect.moveBottom( screen.bottom() );
        if ( rect.top() < screen.top() )
            rect.moveTop( screen.top() );

        if ( rect.right() > screen.right() )
            rect.moveRight( screen.right() );
        if ( rect.left() < screen.left() )
            rect.moveLeft( screen.left() );

        move( rect.topLeft() );
        resize( newSize );
    } else {
        resize( newSize );
    }
}
