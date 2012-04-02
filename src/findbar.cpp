/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2012 Michał Męciński
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

#include "findbar.h"

#include "utils/iconloader.h"

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>

FindBar::FindBar( QWidget* parent ) : QWidget( parent ),
    m_boundWidget( NULL ),
    m_enabled( false )
{
    setWindowTitle( tr( "Find" ) );

    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setMargin( 3 );
    layout->setSpacing( 5 );

    QToolButton* closeButton = new QToolButton( this );
    closeButton->setIcon( IconLoader::icon( "close" ) );
    closeButton->setIconSize( QSize( 16, 16 ) );
    closeButton->setToolTip( tr( "Close" ) );
    closeButton->setAutoRaise( true );
    layout->addWidget( closeButton );

    connect( closeButton, SIGNAL( clicked() ), this, SLOT( hide() ) );

    QLabel* label = new QLabel( tr( "Find:" ), this );
    layout->addWidget( label );

    m_edit = new QLineEdit( this );
    m_edit->setMaximumWidth( 200 );
    layout->addWidget( m_edit, 1 );

    label->setBuddy( m_edit );

    connect( m_edit, SIGNAL( textChanged( const QString& ) ), this, SLOT( textChanged( const QString& ) ) );

    m_edit->installEventFilter( this );

    m_previousButton = new QToolButton( this );
    m_previousButton->setIcon( IconLoader::icon( "find-previous" ) );
    m_previousButton->setIconSize( QSize( 16, 16 ) );
    m_previousButton->setToolTip( tr( "Find Previous" ) );
    m_previousButton->setAutoRaise( true );
    layout->addWidget( m_previousButton );

    connect( m_previousButton, SIGNAL( clicked() ), this, SIGNAL( findPrevious() ) );

    m_nextButton = new QToolButton( this );
    m_nextButton->setIcon( IconLoader::icon( "find-next" ) );
    m_nextButton->setIconSize( QSize( 16, 16 ) );
    m_nextButton->setToolTip( tr( "Find Next" ) );
    m_nextButton->setAutoRaise( true );
    layout->addWidget( m_nextButton );

    connect( m_nextButton, SIGNAL( clicked() ), this, SIGNAL( findNext() ) );

    layout->addSpacing( 5 );

    m_caseCheckBox = new QCheckBox( tr( "&Match case" ), this );
    layout->addWidget( m_caseCheckBox );

    connect( m_caseCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( caseToggled() ) );

    layout->addSpacing( 10 );

    m_warningPixmap = new QLabel( this );
    m_warningPixmap->setPixmap( IconLoader::pixmap( "warning" ) );
    layout->addWidget( m_warningPixmap );

    m_warningLabel = new QLabel( tr( "Text not found" ) );
    layout->addWidget( m_warningLabel );

    layout->addStretch( 0 );

    m_previousButton->setEnabled( false );
    m_nextButton->setEnabled( false );

    m_warningPixmap->hide();
    m_warningLabel->hide();

    setFocusProxy( m_edit );
}

FindBar::~FindBar()
{
}

void FindBar::setText( const QString& text )
{
    bool old = blockSignals( true );
    m_edit->setText( text );
    blockSignals( old );
}

QString FindBar::text() const
{
    return m_edit->text();
}

void FindBar::setFlags( QTextDocument::FindFlags flags )
{
    bool old = blockSignals( true );
    m_caseCheckBox->setChecked( ( flags & QTextDocument::FindCaseSensitively ) != 0 );
    blockSignals( old );
}

QTextDocument::FindFlags FindBar::flags() const
{
    QTextDocument::FindFlags flags = 0;
    if ( m_caseCheckBox->isChecked() )
        flags |= QTextDocument::FindCaseSensitively;
    return flags;
}

void FindBar::showWarning( bool on )
{
    m_warningPixmap->setVisible( on );
    m_warningLabel->setVisible( on );
}

void FindBar::setBoundWidget( QWidget* widget )
{
    m_boundWidget = widget;
}

void FindBar::selectAll()
{
    m_edit->selectAll();
}

void FindBar::hideEvent( QHideEvent* e )
{
    if ( m_boundWidget && !e->spontaneous() )
        m_boundWidget->setFocus();
}

bool FindBar::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == m_edit && e->type() == QEvent::ShortcutOverride ) {
        QKeyEvent* ke = (QKeyEvent*)e;
        if ( ke->key() == Qt::Key_Escape ) {
            hide();
            ke->accept();
            return true;
        }
    }

    if ( obj == m_edit && e->type() == QEvent::KeyPress ) {
        QKeyEvent* ke = (QKeyEvent*)e;
        if ( ke->key() == Qt::Key_Escape ) {
            hide();
            return true;
        }
        if ( ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter ) {
            if ( ke->modifiers() & Qt::ShiftModifier )
                emit findPrevious();
            else
                emit findNext();
            return true;
        }
        if ( m_boundWidget && ( ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down
            || ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown ) ) {
            QApplication::sendEvent( m_boundWidget, ke );
            return true;
        }
    }

    return QWidget::eventFilter( obj, e );
}

void FindBar::textChanged( const QString& text )
{
    bool enabled = !text.isEmpty();

    if ( m_enabled != enabled ) {
        m_enabled = enabled;

        m_previousButton->setEnabled( m_enabled );
        m_nextButton->setEnabled( m_enabled );

        emit findEnabled( m_enabled );
    }

    emit find( text );
}

void FindBar::caseToggled()
{
    if ( m_enabled )
        emit find( m_edit->text() );
}
