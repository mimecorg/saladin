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

#include "binaryview.h"
#include "binaryloader.h"

#include "application.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "viewer/textedit.h"
#include "xmlui/builder.h"

BinaryView::BinaryView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_loader( NULL ),
    m_hexMode( false )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "hex-mode" ), tr( "Hex Mode" ), this );
    action->setShortcut( Qt::Key_H );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( toggleHexMode() ) );
    setAction( "hexMode", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy" ), this );
    action->setShortcut( QKeySequence::Copy );
    connect( action, SIGNAL( triggered() ), this, SLOT( copy() ) );
    setAction( "copy", action );

    action = new QAction( tr( "Select &All" ), this );
    action->setShortcut( QKeySequence::SelectAll );
    connect( action, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
    setAction( "selectAll", action );

    loadXmlUiFile( ":/resources/binaryview.xml" );

    QWidget* main = new QWidget( parentWidget );
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setContentsMargins( 3, 0, 3, 0 );
    mainLayout->setSpacing( 0 );

    m_edit = new TextEdit( main );
    m_edit->setReadOnly( true );
    m_edit->setWordWrapMode( QTextOption::NoWrap );
    m_edit->setContextMenuPolicy( Qt::CustomContextMenu );

    m_edit->setFont( QFont( "Courier New", 10 ) );

    QPalette palette = m_edit->palette();
    palette.setBrush( QPalette::Inactive, QPalette::Highlight, palette.brush( QPalette::Active, QPalette::Highlight ) );
    palette.setBrush( QPalette::Inactive, QPalette::HighlightedText, palette.brush( QPalette::Active, QPalette::HighlightedText ) );
    m_edit->setPalette( palette );

    m_edit->document()->setDocumentMargin( 0 );

    mainLayout->addWidget( m_edit );

    main->setFocusProxy( m_edit );

    setMainWidget( main );

    connect( m_edit, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( contextMenuRequested( const QPoint& ) ) );
    connect( m_edit, SIGNAL( selectionChanged() ), this, SLOT( updateActions() ) );

    setStatus( tr( "Binary" ) );

    initializeSettings();

    connect( application->applicationSettings(), SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    settingsChanged();

    updateActions();
}

BinaryView::~BinaryView()
{
    if ( m_loader ) {
        m_loader->abort();
        m_loader = NULL;
    }

    storeSettings();
}

View::Type BinaryView::type() const
{
    return Binary;
}

void BinaryView::settingsChanged()
{
    LocalSettings* settings = application->applicationSettings();

    QString family = settings->value( "BinaryFont" ).toString();
    int size = settings->value( "BinaryFontSize" ).toInt();

    QFont font( family, size );
    font.setStyleHint( QFont::Courier );

    m_edit->setFont( font );
}

void BinaryView::initializeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    m_hexMode = settings->value( "HexMode", false ).toBool();
    action( "hexMode" )->setChecked( m_hexMode );
}

void BinaryView::storeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "HexMode", m_hexMode );
}

void BinaryView::load()
{
    if ( m_loader ) {
        m_loader->abort();
        m_loader = NULL;
    }

    m_edit->clear();

    m_length = 0;

    m_loader = new BinaryLoader( path(), m_hexMode );
    m_loader->start();

    connect( m_loader, SIGNAL( nextBlockAvailable() ), this, SLOT( loadNextBlock() ), Qt::QueuedConnection );

    QTimer::singleShot( 0, this, SLOT( loadNextBlock() ) );

    setStatus( tr( "Binary" ) );
}

void BinaryView::loadNextBlock()
{
    QTextCursor cursor = m_edit->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition( QTextCursor::End );

    bool runTimer = true;
    bool atEnd = false;
    quint64 size = 0;

    int count = m_hexMode ? 16 : 64;

    for ( int i = 0; i < count; i++ ) {
        QString block = m_loader->nextBlock();

        if ( block.isEmpty() ) {
            runTimer = false;
            atEnd = m_loader->atEnd();
            size = m_loader->size();
            break;
        }

        cursor.insertText( block );

        m_length += block.length();
    }

    cursor.endEditBlock();

    qint64 estimatedLength = m_loader->estimatedLength();

    if ( atEnd )
        setStatus( tr( "Binary" ) + " (" + tr( "%1 bytes" ).arg( QLocale::system().toString( size ) ) + ")" );
    else if ( estimatedLength > 0 )
        setStatus( tr( "Binary" ) + " (" + tr( "loading... %1%" ).arg( (int)( m_length * 100 / estimatedLength ) ) + ")" );

    if ( runTimer )
        QTimer::singleShot( 0, this, SLOT( loadNextBlock() ) );
}

void BinaryView::updateActions()
{
    bool hasSelection = m_edit->textCursor().hasSelection();

    action( "copy" )->setEnabled( hasSelection );
}

void BinaryView::copy()
{
    m_edit->copy();
}

void BinaryView::selectAll()
{
    m_edit->selectAll();
}

void BinaryView::toggleHexMode()
{
    m_hexMode = action( "hexMode" )->isChecked();

    load();
}

void BinaryView::contextMenuRequested( const QPoint& pos )
{
    QMenu* menu = builder()->contextMenu( "menuContext" );
    if ( menu )
        menu->popup( m_edit->mapToGlobal( pos ) );
}
