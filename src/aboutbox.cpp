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

#include "aboutbox.h"
#include "application.h"

#include "xmlui/gradientwidget.h"

AboutBox::AboutBox( const QString& title, const QString& message, QWidget* parent ) : QDialog( parent )
{
    setWindowTitle( title );

    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    XmlUi::GradientWidget* promptWidget = new XmlUi::GradientWidget( this );
    topLayout->addWidget( promptWidget );

    QHBoxLayout* promptLayout = new QHBoxLayout( promptWidget );
    promptLayout->setSpacing( 10 );

    QLabel* promptPixmap = new QLabel( promptWidget );
    promptPixmap->setPixmap( application->windowIcon().pixmap( QSize( 48, 48 ) ) );
    promptLayout->addWidget( promptPixmap, 0, Qt::AlignTop | Qt::AlignLeft );

    QLabel* promptLabel = new QLabel( promptWidget );
    promptLabel->setWordWrap( true );
    promptLabel->setText( message );
    promptLabel->setMinimumWidth( 350 );
    promptLayout->addWidget( promptLabel, 1, Qt::AlignTop );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    topLayout->addWidget( separator );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin( 9 );
    mainLayout->setSpacing( 4 );
    topLayout->addLayout( mainLayout );

    AboutBoxScrollArea* scrollArea = new AboutBoxScrollArea( this );
    mainLayout->addWidget( scrollArea );

    scrollArea->setBackgroundRole( QPalette::Base );

    m_sectionsWidget = new QWidget( scrollArea );
    scrollArea->setWidget( m_sectionsWidget );
    scrollArea->setWidgetResizable( true );

    m_sectionsLayout = new QVBoxLayout( m_sectionsWidget );
    m_sectionsLayout->setMargin( 6 );
    m_sectionsLayout->setSpacing( 6 );

    m_sectionsLayout->addStretch( 1 );

    mainLayout->addSpacing( 7 );

    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok, Qt::Horizontal, this );
    mainLayout->addWidget( buttonBox );

    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );

    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
}

AboutBox::~AboutBox()
{
}

AboutBoxSection* AboutBox::addSection( const QPixmap& pixmap, const QString& message )
{
    AboutBoxSection* section = new AboutBoxSection( pixmap, message, m_sectionsWidget );

    m_sectionsLayout->insertWidget( m_sectionsLayout->count() - 1, section );

    return section;
}

AboutBoxSection::AboutBoxSection( const QPixmap& pixmap, const QString& message, QWidget* parent ) : QFrame( parent ),
    m_buttonsLayout( NULL )
{
    setFrameStyle( Box | Sunken );

    initialize();

    m_pixmapLabel->setPixmap( pixmap );
    m_messageLabel->setText( message );
}

AboutBoxSection::AboutBoxSection( Qt::WindowFlags flags ) : QFrame( NULL, flags ),
    m_buttonsLayout( NULL )
{
    initialize();
}

void AboutBoxSection::initialize()
{
    m_mainLayout = new QHBoxLayout( this );
    m_mainLayout->setSpacing( 10 );

    m_pixmapLabel = new QLabel( this );
    m_mainLayout->addWidget( m_pixmapLabel, 0, Qt::AlignTop | Qt::AlignLeft );

    m_messageLabel = new QLabel( this );
    m_messageLabel->setWordWrap( true );
    m_messageLabel->setOpenExternalLinks( true );
    m_mainLayout->addWidget( m_messageLabel, 1, Qt::AlignTop );
}

AboutBoxSection::~AboutBoxSection()
{
}

void AboutBoxSection::setPixmap( const QPixmap& pixmap )
{
    m_pixmapLabel->setPixmap( pixmap );
}

void AboutBoxSection::setMessage( const QString& message )
{
    m_messageLabel->setText( message );
}

QPushButton* AboutBoxSection::addButton( const QString& text )
{
    if ( !m_buttonsLayout ) {
        m_buttonsLayout = new QHBoxLayout();
        m_buttonsLayout->setSpacing( 6 );
        m_mainLayout->addLayout( m_buttonsLayout );
    }

    QPushButton* button = new QPushButton( text, this );
    m_buttonsLayout->addWidget( button, 0, Qt::AlignBottom );

    return button;
}

void AboutBoxSection::clearButtons()
{
    if ( !m_buttonsLayout )
        return;

    while ( m_buttonsLayout->count() > 0 )
        delete m_buttonsLayout->itemAt( 0 )->widget();

    delete m_buttonsLayout;
    m_buttonsLayout = NULL;
}

AboutBoxToolSection::AboutBoxToolSection() : AboutBoxSection( Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint )
{
    setAttribute( Qt::WA_DeleteOnClose, true );

    setFrameStyle( Panel | Plain );

    setBackgroundRole( QPalette::ToolTipBase );
    setAutoFillBackground( true );
}

AboutBoxToolSection::~AboutBoxToolSection()
{
}

void AboutBoxToolSection::updatePosition()
{
    QDesktopWidget desktop;
    QRect screen = desktop.screenGeometry();
    QRect available = desktop.availableGeometry();

    int w = 450;
    int h = sizeHint().height();
    resize( w, h );

    if ( available.left() > screen.left() )
        move( available.left(), available.bottom() - h + 1 );
    else
        move( available.right() - w, available.bottom() - h + 1 );
}

void AboutBoxToolSection::addCloseButton( const QIcon& icon )
{
    QToolButton* button = new QToolButton( this );
    button->setAutoRaise( true );
    button->setIconSize( QSize( 16, 16 ) );
    button->setIcon( icon );
    button->setToolTip( tr( "Close" ) );

    button->resize( button->sizeHint() );
    button->move( width() - button->width() - 2, 2 );

    connect( button, SIGNAL( clicked() ), this, SLOT( close() ) );
}

void AboutBoxToolSection::setMessage( const QString& message )
{
    AboutBoxSection::setMessage( message );

    updatePosition();
}

AboutBoxScrollArea::AboutBoxScrollArea( QWidget* parent ) : QScrollArea( parent )
{
}

AboutBoxScrollArea::~AboutBoxScrollArea()
{
}

QSize AboutBoxScrollArea::sizeHint() const
{
    int w = 475;
    int fw = frameWidth();
    return QSize( w, widget()->heightForWidth( w - 2 * fw ) + 2 * fw );
}
