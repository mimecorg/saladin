/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011 Michał Męciński
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

#include "application.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "viewer/textedit.h"
#include "xmlui/builder.h"

BinaryView::BinaryView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_hexMode( false ),
    m_lastHexMode( false )
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

    setMainWidget( main );

    connect( m_edit, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( contextMenuRequested( const QPoint& ) ) );
    connect( m_edit, SIGNAL( selectionChanged() ), this, SLOT( updateActions() ) );

    setStatus( tr( "Binary" ) );

    initializeSettings();

    updateActions();
}

BinaryView::~BinaryView()
{
    storeSettings();
}

View::Type BinaryView::type() const
{
    return Binary;
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
    QString status = tr( "Binary" );

    int topLine = m_edit->cursorForPosition( QPoint( 0, 0 ) ).blockNumber();
    int topPos = topLine * ( m_lastHexMode ? 16 : 80 );

    m_edit->clear();

    QFile file( path() );

    if ( file.open( QIODevice::ReadOnly ) ) {
        QDataStream stream( &file );

        char input[ 256 ];
        for ( int i = 0; i < 256; i++ )
            input[ i ] = i;

        QTextCodec* codec = QTextCodec::codecForLocale();
        QString output = codec->toUnicode( input, 256 );
        QChar* map = output.data();

        for ( int i = 0; i < 256; i++ ) {
            if ( !map[ i ].isPrint() )
                map[ i ] = QLatin1Char( '.' );
            else if ( map[ i ].unicode() == 173 )
                map[ i ] = QLatin1Char( '-' );
        }

        int readSize = m_hexMode ? 16 : 80;
        int offset = m_hexMode ? 61 : 0;

        int lines = (int)( file.size() / readSize ) + 2;

        QString text;
        text.reserve( lines * ( readSize + offset + 1 ) );

        uint pos = 0;

        char buffer[ 80 ];
        int len;

        QChar hex[ 16 ];
        QChar line[ 81 ];

        if ( m_hexMode ) {
            for ( int i = 0; i < 10; i++ )
                hex[ i ] = QLatin1Char( i + '0' );
            for ( int i = 0; i < 6; i++ )
                hex[ i + 10 ] = QLatin1Char( i + 'A' );

            for ( int i = 0; i < 81; i++ )
                line[ i ] = QLatin1Char( ' ' );
            line[ 9 ] = QLatin1Char( '|' );
            line[ 59 ] = QLatin1Char( '|' );
        }

        while ( ( len = stream.readRawData( buffer, readSize ) ) > 0 ) {
            if ( m_hexMode ) {
                uint p = pos;
                for ( int i = 7; i >= 0; i-- ) {
                    line[ i ] = hex[ p & 0xf ];
                    p >>= 4;
                }

                for ( int i = 0; i < len; i++ ) {
                    uchar ch = (uchar)buffer[ i ];
                    line[ 3 * i + 11 ] = hex[ ( ch >> 4 ) & 0xf ];
                    line[ 3 * i + 12 ] = hex[ ch & 0xf ];
                }

                for ( int i = len; i < 16; i++ ) {
                    line[ 3 * i + 11 ] = QLatin1Char( ' ' );
                    line[ 3 * i + 12 ] = QLatin1Char( ' ' );
                }
            }

            for ( int i = 0; i < len; i++ )
                line[ i + offset ] = map[ (uchar)buffer[ i ] ];
            line[ len + offset ] = QLatin1Char( '\n' );

            text.append( QString::fromRawData( line, len + offset + 1 ) );

            pos += len;
        }

        m_edit->setPlainText( text );

        topLine = topPos / readSize;

        if ( topLine > 0 ) {
            m_edit->verticalScrollBar()->triggerAction( QAbstractSlider::SliderToMaximum );

            QTextCursor cursor = m_edit->textCursor();
            cursor.movePosition( QTextCursor::Start );
            cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor, topLine );
            m_edit->setTextCursor( cursor );
            m_edit->ensureCursorVisible();
        }

        m_lastHexMode = m_hexMode;

        status += " (" + tr( "%1 bytes" ).arg( QLocale::system().toString( pos ) ) + ")";
    }

    setStatus( status );
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
