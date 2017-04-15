/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2017 Michał Męciński
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

#include "gotodialog.h"

#include "utils/iconloader.h"
#include "xmlui/gradientwidget.h"

GoToDialog::GoToDialog( int lines, int current, QWidget* parent ) : QDialog( parent )
{
    setWindowTitle( tr( "Go To Line" ) );

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

    QBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin( 9 );
    mainLayout->setSpacing( 6 );
    topLayout->addLayout( mainLayout );

    QBoxLayout* lineLayout = new QHBoxLayout();
    mainLayout->addLayout( lineLayout );

    QLabel* label = new QLabel( tr( "&Line number:" ), this );
    lineLayout->addWidget( label );

    m_spinBox = new QSpinBox( this );
    m_spinBox->setRange( 1, lines );
    m_spinBox->setValue( current );
    m_spinBox->setFixedWidth( 150 );
    lineLayout->addWidget( m_spinBox );

    label->setBuddy( m_spinBox );

    lineLayout->addStretch( 1 );

    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
    mainLayout->addWidget( buttonBox );

    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );
    buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );

    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    promptPixmap->setPixmap( IconLoader::pixmap( "goto", 22 ) );

    promptLabel->setText( tr( "Go to the specified line (1 - %1):" ).arg( lines ) );
    promptLabel->setMinimumWidth( 350 );
    promptLabel->setFixedHeight( promptLabel->heightForWidth( 350 ) );

    setMaximumHeight( sizeHint().height() );

    m_spinBox->selectAll();
}

GoToDialog::~GoToDialog()
{
}

int GoToDialog::line() const
{
    return m_spinBox->value();
}
