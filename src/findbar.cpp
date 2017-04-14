/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2014 Michał Męciński
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

#include "application.h"
#include "utils/localsettings.h"
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

    m_closeButton = new QToolButton( this );
    m_closeButton->setIconSize( QSize( 16, 16 ) );
    m_closeButton->setToolTip( tr( "Close" ) );
    m_closeButton->setAutoRaise( true );
    layout->addWidget( m_closeButton );

    connect( m_closeButton, SIGNAL( clicked() ), this, SLOT( hide() ) );

    QLabel* label = new QLabel( tr( "Find:" ), this );
    layout->addWidget( label );

    m_comboBox = new QComboBox( this );
    m_comboBox->setEditable( true );
    m_comboBox->setCompleter( NULL );
    m_comboBox->setInsertPolicy( QComboBox::NoInsert );
    m_comboBox->setMaximumWidth( 200 );
    layout->addWidget( m_comboBox, 1 );

    label->setBuddy( m_comboBox );

    connect( m_comboBox, SIGNAL( currentTextChanged( const QString& ) ), this, SLOT( textChanged( const QString& ) ) );

    m_comboBox->installEventFilter( this );

    m_previousButton = new QToolButton( this );
    m_previousButton->setIconSize( QSize( 16, 16 ) );
    m_previousButton->setToolTip( QString( "%1 (Shift+F3)" ).arg( tr( "Find Previous" ) ) );
    m_previousButton->setAutoRaise( true );
    layout->addWidget( m_previousButton );

    connect( m_previousButton, SIGNAL( clicked() ), this, SIGNAL( findPrevious() ) );

    m_nextButton = new QToolButton( this );
    m_nextButton->setIconSize( QSize( 16, 16 ) );
    m_nextButton->setToolTip( QString( "%1 (F3)" ).arg( tr( "Find Next" ) ) );
    m_nextButton->setAutoRaise( true );
    layout->addWidget( m_nextButton );

    connect( m_nextButton, SIGNAL( clicked() ), this, SIGNAL( findNext() ) );

    layout->addSpacing( 5 );

    m_caseCheckBox = new QCheckBox( tr( "&Match case" ), this );
    layout->addWidget( m_caseCheckBox );

    connect( m_caseCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( caseToggled() ) );

    layout->addSpacing( 10 );

    m_warningPixmap = new QLabel( this );
    layout->addWidget( m_warningPixmap );

    m_warningLabel = new QLabel( tr( "Text not found" ) );
    layout->addWidget( m_warningLabel );

    layout->addStretch( 0 );

    loadIcons();

    connect( application, SIGNAL( themeChanged() ), this, SLOT( loadIcons() ) );

    m_previousButton->setEnabled( false );
    m_nextButton->setEnabled( false );

    m_warningPixmap->hide();
    m_warningLabel->hide();

    setFocusProxy( m_comboBox );
}

FindBar::~FindBar()
{
}

QString FindBar::text() const
{
    return m_comboBox->currentText();
}

void FindBar::setTextList( const QStringList& list )
{
    bool old = blockSignals( true );
    m_comboBox->clear();
    m_comboBox->addItems( list );
    m_comboBox->setCurrentIndex( 0 );
    m_comboBox->lineEdit()->selectAll();
    blockSignals( old );
}

QStringList FindBar::textList() const
{
    QStringList list;
    list.append( m_comboBox->currentText() );
    for ( int i = 0; i < m_comboBox->count(); i++ ) {
        QString item = m_comboBox->itemText( i );
        if ( item != list.first() && list.count() < 10 )
            list.append( item );
    }
    return list;
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
    m_comboBox->lineEdit()->selectAll();
}

void FindBar::hideEvent( QHideEvent* e )
{
    if ( m_boundWidget && !e->spontaneous() )
        m_boundWidget->setFocus();
}

bool FindBar::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == m_comboBox && e->type() == QEvent::ShortcutOverride ) {
        QKeyEvent* ke = (QKeyEvent*)e;
        if ( ke->key() == Qt::Key_Escape ) {
            hide();
            ke->accept();
            return true;
        }
    }

    if ( obj == m_comboBox && e->type() == QEvent::KeyPress ) {
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
        emit find( m_comboBox->currentText() );
}

void FindBar::loadIcons()
{
    m_closeButton->setIcon( IconLoader::icon( "close" ) );
    m_previousButton->setIcon( IconLoader::icon( "find-previous" ) );
    m_nextButton->setIcon( IconLoader::icon( "find-next" ) );

    m_warningPixmap->setPixmap( IconLoader::pixmap( "warning" ) );
}
