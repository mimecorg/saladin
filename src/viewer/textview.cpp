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

#include "textview.h"
#include "textloader.h"

#include "application.h"
#include "mainwindow.h"
#include "findbar.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "viewer/textedit.h"
#include "viewer/gotodialog.h"
#include "xmlui/toolstrip.h"
#include "xmlui/builder.h"

TextView::TextView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_loader( NULL ),
    m_length( 0 ),
    m_isFindEnabled( false ),
    m_currentLine( 0 )
{
    QAction* action;
    XmlUi::ToolStripAction* encodingAction;

    action = new QAction( tr( "Line Numbers" ), this );
    action->setShortcut( Qt::Key_L );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( toggleLineNumbers() ) );
    setAction( "lineNumbers", action );

    action = new QAction( tr( "Word Wrap" ), this );
    action->setShortcut( Qt::Key_W );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( toggleWordWrap() ) );
    setAction( "wordWrap", action );

    encodingAction = new XmlUi::ToolStripAction( tr( "Encoding" ), this );
    encodingAction->setShortcut( Qt::Key_E );
    encodingAction->setPopupMode( QToolButton::InstantPopup );
    connect( encodingAction, SIGNAL( triggered() ), this, SLOT( selectEncoding() ) );
    setAction( "selectEncoding", encodingAction );

    action = new QAction( tr( "&Copy" ), this );
    action->setShortcut( QKeySequence::Copy );
    connect( action, SIGNAL( triggered() ), this, SLOT( copy() ) );
    setAction( "copy", action );

    action = new QAction( tr( "Select &All" ), this );
    action->setShortcut( QKeySequence::SelectAll );
    connect( action, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
    setAction( "selectAll", action );

    action = new QAction( tr( "&Find..." ), this );
    action->setShortcut( QKeySequence::Find );
    connect( action, SIGNAL( triggered() ), this, SLOT( find() ) );
    setAction( "find", action );

    action = new QAction( tr( "Find &Next" ), this );
    action->setShortcut( QKeySequence::FindNext );
    connect( action, SIGNAL( triggered() ), this, SLOT( findNext() ) );
    setAction( "findNext", action );

    action = new QAction( tr( "Find &Previous" ), this );
    action->setShortcut( QKeySequence::FindPrevious );
    connect( action, SIGNAL( triggered() ), this, SLOT( findPrevious() ) );
    setAction( "findPrevious", action );

    action = new QAction( tr( "&Go To Line..." ), this );
    action->setShortcut( Qt::CTRL + Qt::Key_G );
    connect( action, SIGNAL( triggered() ), this, SLOT( goToLine() ) );
    setAction( "goToLine", action );

    action = new QAction( tr( "Edit File" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F4 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editFile() ) );
    setAction( "editFile", action );

    loadIcons();

    connect( application, SIGNAL( themeChanged() ), this, SLOT( loadIcons() ) );

    loadXmlUiFile( ":/resources/textview.xml" );

    QWidget* main = new QWidget( parentWidget );
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );

    QVBoxLayout* editLayout = new QVBoxLayout();
    editLayout->setContentsMargins( 3, 0, 3, 0 );
    mainLayout->addLayout( editLayout );

    m_edit = new TextEdit( main );
    m_edit->setReadOnly( true );
    m_edit->setContextMenuPolicy( Qt::CustomContextMenu );

    QFont font( "Courier New", 10 );
    m_edit->setFont( font );

    QFontMetrics metrics( font );
    m_edit->setTabStopWidth( 4 * metrics.width( ' ' ) );

    QPalette palette = m_edit->palette();
    palette.setBrush( QPalette::Inactive, QPalette::Highlight, palette.brush( QPalette::Active, QPalette::Highlight ) );
    palette.setBrush( QPalette::Inactive, QPalette::HighlightedText, palette.brush( QPalette::Active, QPalette::HighlightedText ) );
    m_edit->setPalette( palette );

    m_edit->document()->setDocumentMargin( 0 );

    editLayout->addWidget( m_edit );

    connect( m_edit, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( contextMenuRequested( const QPoint& ) ) );
    connect( m_edit, SIGNAL( selectionChanged() ), this, SLOT( updateActions() ) );

    m_findBar = new FindBar( main );
    m_findBar->setBoundWidget( m_edit );
    m_findBar->setAutoFillBackground( true );
    m_findBar->hide();

    mainLayout->addWidget( m_findBar );

    connect( m_findBar, SIGNAL( find( const QString& ) ), this, SLOT( findText( const QString& ) ) );
    connect( m_findBar, SIGNAL( findPrevious() ), this, SLOT( findPrevious() ) );
    connect( m_findBar, SIGNAL( findNext() ), this, SLOT( findNext() ) );
    connect( m_findBar, SIGNAL( findEnabled( bool ) ), this, SLOT( updateActions() ) );

    main->setFocusProxy( m_edit );

    setMainWidget( main );

    setStatus( tr( "Text" ) );

    main->installEventFilter( this );

    m_encodingMapper = new QSignalMapper( this );
    connect( m_encodingMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( setEncoding( const QString& ) ) );

    encodingAction->setMenu( createEncodingMenu() );

    initializeSettings();

    connect( application->applicationSettings(), SIGNAL( settingsChanged() ), this, SLOT( settingsChanged() ) );

    settingsChanged();

    updateActions();
}

TextView::~TextView()
{
    if ( m_loader ) {
        m_loader->abort();
        m_loader = NULL;
    }

    storeSettings();
}

void TextView::settingsChanged()
{
    LocalSettings* settings = application->applicationSettings();

    QString family = settings->value( "TextFont" ).toString();
    int size = settings->value( "TextFontSize" ).toInt();

    QFont font( family, size );
    font.setStyleHint( QFont::Courier );

    m_edit->setFont( font );
}

void TextView::initializeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    bool wordWrap = settings->value( "WordWrap", true ).toBool();
    m_edit->setWordWrapMode( wordWrap ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap );
    action( "wordWrap" )->setChecked( wordWrap );

    bool lineNumbers = settings->value( "LineNumbers", false ).toBool();
    m_edit->setLineNumbers( lineNumbers );
    action( "lineNumbers" )->setChecked( lineNumbers );

    QStringList list = settings->value( "FindText" ).toStringList();
    QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings->value( "FindFlags" ).toInt();
    m_findBar->setTextList( list );
    m_findBar->setFlags( flags );
}

void TextView::storeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "WordWrap", action( "wordWrap" )->isChecked() );
    settings->setValue( "LineNumbers", action( "lineNumbers" )->isChecked() );

    settings->setValue( "FindText", m_findBar->textList() );
    settings->setValue( "FindFlags", (int)m_findBar->flags() );
}

QMenu* TextView::createEncodingMenu()
{
    QMenu* menu = new QMenu( m_edit );

    struct Encoding
    {
        const char* m_format;
        const char* m_name;
        int m_key;
    };

    Encoding stdEncodings[] = {
        { "System",   QT_TR_NOOP( "ANSI" ),       Qt::Key_A },
        { "IBM850",   QT_TR_NOOP( "OEM" ),        Qt::Key_O },
        { "UTF-8",    QT_TR_NOOP( "UTF-8" ),      Qt::Key_U },
        { "UTF-16LE", QT_TR_NOOP( "Unicode" ),    0 },
        { "UTF-16BE", QT_TR_NOOP( "Unicode BE" ), 0 },
    };

    for ( int i = 0; i < sizeof( stdEncodings ) / sizeof( stdEncodings[ 0 ] ); i++ ) {
        QAction* action = new QAction( tr( stdEncodings[ i ].m_name ), this );
        action->setCheckable( true );

        if ( stdEncodings[ i ].m_key != 0 )
            action->setShortcut( stdEncodings[ i ].m_key );

        connect( action, SIGNAL( triggered() ), m_encodingMapper, SLOT( map() ) );
        m_encodingMapper->setMapping( action, stdEncodings[ i ].m_format );

        menu->addAction( action );
    }

    menu->addSeparator();

    QList<QAction*> actions = menu->actions();
    int startIndex = actions.count();

    QStringList names;

    Encoding extEncodings[] = {
        { "Windows-1250", QT_TR_NOOP( "Central European" ) },
        { "Windows-1251", QT_TR_NOOP( "Cyrillic" ) },
        { "Windows-1252", QT_TR_NOOP( "Western European" ) },
        { "Windows-1253", QT_TR_NOOP( "Greek" ) },
        { "Windows-1254", QT_TR_NOOP( "Turkish" ) },
        { "Windows-1255", QT_TR_NOOP( "Hebrew" ) },
        { "Windows-1256", QT_TR_NOOP( "Arabic" ) },
        { "Windows-1257", QT_TR_NOOP( "Baltic" ) },
        { "Windows-1258", QT_TR_NOOP( "Vietnamese" ) },
        { "ISO-8859-1",   QT_TR_NOOP( "Western European" ) },
        { "ISO-8859-2",   QT_TR_NOOP( "Central European" ) },
        { "ISO-8859-3",   QT_TR_NOOP( "South European" ) },
        { "ISO-8859-4",   QT_TR_NOOP( "North European" ) },
        { "ISO-8859-5",   QT_TR_NOOP( "Cyrillic" ) },
        { "ISO-8859-6",   QT_TR_NOOP( "Arabic" ) },
        { "ISO-8859-7",   QT_TR_NOOP( "Greek" ) },
        { "ISO-8859-8",   QT_TR_NOOP( "Hebrew" ) },
        { "ISO-8859-9",   QT_TR_NOOP( "Turkish" ) },
        { "ISO-8859-13",  QT_TR_NOOP( "Baltic" ) },
        { "ISO-8859-14",  QT_TR_NOOP( "Celtic" ) },
        { "ISO-8859-15",  QT_TR_NOOP( "Western European" ) },
        { "ISO-8859-16",  QT_TR_NOOP( "Romanian" ) },
        { "KOI8-R",       QT_TR_NOOP( "Cyrillic" ) },
        { "KOI8-U",       QT_TR_NOOP( "Cyrillic" ) },
        { "IBM866",       QT_TR_NOOP( "Cyrillic" ) },
        { "GB18030",      QT_TR_NOOP( "Chinese Simplified" ) },
        { "Big5",         QT_TR_NOOP( "Chinese Traditional" ) },
        { "Big5-HKSCS",   QT_TR_NOOP( "Chinese Traditional" ) },
        { "Shift_JIS",    QT_TR_NOOP( "Japanese" ) },
        { "EUC-JP",       QT_TR_NOOP( "Japanese" ) },
        { "ISO-2022-JP",  QT_TR_NOOP( "Japanese" ) },
        { "EUC-KR",       QT_TR_NOOP( "Korean" ) },
        { "TIS-620",      QT_TR_NOOP( "Thai" ) },
    };

    for ( int i = 0; i < sizeof( extEncodings ) / sizeof( extEncodings[ 0 ] ); i++ ) {
        QString name = tr( extEncodings[ i ].m_name );
        QString text = QString( "%1 (%2)" ).arg( name, extEncodings[ i ].m_format );

        QAction* action = new QAction( text, this );
        action->setCheckable( true );

        connect( action, SIGNAL( triggered() ), m_encodingMapper, SLOT( map() ) );
        m_encodingMapper->setMapping( action, extEncodings[ i ].m_format );

        int index = 0;
        while ( index < names.count() && QString::localeAwareCompare( name, names.at( index ) ) >= 0 )
            index++;

        menu->insertAction( actions.value( startIndex + index ), action );

        names.insert( index, name );
        actions.insert( startIndex + index, action );
    }

    return menu;
}

View::Type TextView::type() const
{
    return Text;
}

void TextView::load()
{
    if ( m_loader ) {
        m_loader->abort();
        m_loader = NULL;
    }

    m_edit->clear();

    m_length = 0;
    m_currentLine = 0;

    QObject* current = m_encodingMapper->mapping( QString( format() ) );

    QList<QAction*> actions = action( "selectEncoding" )->menu()->actions();
    foreach ( QAction* action, actions ) {
        if ( action->isCheckable() )
            action->setChecked( action == current );
        if ( action == current )
            m_encoding = action->text();
    }

    m_loader = new TextLoader( pidl(), format() );
    m_loader->start();

    connect( m_loader, SIGNAL( nextBlockAvailable() ), this, SLOT( loadNextBlock() ), Qt::QueuedConnection );

    QTimer::singleShot( 0, this, SLOT( loadNextBlock() ) );

    setStatus( tr( "Text" ) + ", " + m_encoding );
}

void TextView::loadNextBlock()
{
    QTextCursor cursor = m_edit->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition( QTextCursor::End );

    bool runTimer = true;
    bool atEnd = false;

    for ( int i = 0; i < 64; i++ ) {
        QString block = m_loader->nextBlock();

        if ( block.isEmpty() ) {
            runTimer = false;
            atEnd = m_loader->atEnd();
            break;
        }

        cursor.insertText( block );

        m_length += block.length();
    }

    cursor.endEditBlock();

    qint64 estimatedLength = m_loader->estimatedLength();

    if ( atEnd )
        setStatus( tr( "Text" ) + ", " + m_encoding + " (" + tr( "%1 characters, %2 lines" ).arg( QLocale::system().toString( m_length ), QLocale::system().toString( m_edit->document()->blockCount() ) ) + ")" );
    else if ( estimatedLength > 0 )
        setStatus( tr( "Text" ) + ", " + m_encoding + " (" + tr( "loading... %1%" ).arg( (int)( m_length * 100 / estimatedLength ) ) + ")" );

    if ( runTimer )
        QTimer::singleShot( 0, this, SLOT( loadNextBlock() ) );
}

void TextView::updateActions()
{
    m_isFindEnabled = m_findBar->isFindEnabled();

    m_currentLine = m_edit->textCursor().blockNumber() + 1;

    bool hasSelection = m_edit->textCursor().hasSelection();

    action( "copy" )->setEnabled( hasSelection );
    action( "findNext" )->setEnabled( m_isFindEnabled );
    action( "findPrevious" )->setEnabled( m_isFindEnabled );
}

void TextView::copy()
{
    m_edit->copy();
}

void TextView::selectAll()
{
    m_edit->selectAll();
}

void TextView::find()
{
    m_findBar->show();
    m_findBar->setFocus();
    m_findBar->selectAll();
}

void TextView::findText( const QString& text )
{
    findText( text, m_edit->textCursor().selectionStart(), m_findBar->flags() );
}

void TextView::findNext()
{
    if ( m_isFindEnabled ) {
        findText( m_findBar->text(), m_edit->textCursor().selectionStart() + 1, m_findBar->flags() );
        m_findBar->setTextList( m_findBar->textList() );
    }
}

void TextView::findPrevious()
{
    if ( m_isFindEnabled ) {
        findText( m_findBar->text(), m_edit->textCursor().selectionStart(), m_findBar->flags() | QTextDocument::FindBackward );
        m_findBar->setTextList( m_findBar->textList() );
    }
}

void TextView::findText( const QString& text, int from, QTextDocument::FindFlags flags )
{
    QTextCursor found;
    bool warn = false;

    if ( !text.isEmpty() ) {
        found = m_edit->document()->find( text, from, flags );

        if ( found.isNull() ) {
            if ( flags & QTextDocument::FindBackward ) {
                QTextCursor end( m_edit->document() );
                end.movePosition( QTextCursor::End );
                from = end.position();
            } else {
                from = 0;
            }

            found = m_edit->document()->find( text, from, flags );

            if ( found.isNull() )
                warn = true;
        }
    }

    m_findBar->show();
    m_findBar->setFocus();

    if ( !found.isNull() ) {
        m_edit->setTextCursor( found );
        updateActions();
    }

    m_findBar->showWarning( warn );
}

void TextView::toggleLineNumbers()
{
    m_edit->setLineNumbers( action( "lineNumbers" )->isChecked() );
}

void TextView::goToLine()
{
    GoToDialog dialog( m_edit->document()->blockCount(), m_currentLine, mainWidget() );

    if ( dialog.exec() == QDialog::Accepted ) {
        QTextCursor cursor( m_edit->document() );
        if ( dialog.line() < m_edit->document()->blockCount() ) {
            cursor.movePosition( QTextCursor::Start );
            cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor, dialog.line() );
            cursor.movePosition( QTextCursor::PreviousBlock, QTextCursor::KeepAnchor );
        } else {
            cursor.movePosition( QTextCursor::End );
            cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::KeepAnchor );
        }
        m_edit->setTextCursor( cursor );
        m_edit->centerCursor();
    }
}

void TextView::toggleWordWrap()
{
    m_edit->setWordWrapMode( action( "wordWrap" )->isChecked() ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap );
}

void TextView::setEncoding( const QString& format )
{
    setFormat( format.toLatin1() );

    load();
}

void TextView::selectEncoding()
{
    builder()->toolStrip( "stripMain" )->execMenu( action( "selectEncoding" ) );
}

void TextView::editFile()
{
    if ( pidl().isValid() && pidl().attributes().testFlag( ShellItem::FileSystem ) ) {
        QString path = pidl().path();
        if ( path.isEmpty() )
            return;

        mainWindow->startTool( MainWindow::EditorTool, QString( "\"%1\"" ).arg( path ), QFileInfo( path ).absolutePath() );
    }
}

bool TextView::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == mainWidget() ) {
        if ( e->type() == QEvent::ShortcutOverride ) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if ( ke->key() == Qt::Key_F3 && ( ke->modifiers() & ~Qt::ShiftModifier ) == 0 ) {
                if ( !m_isFindEnabled ) {
                    find();
                    ke->accept();
                    return true;
                }
            }
        }
    }
    return View::eventFilter( obj, e );
}

void TextView::contextMenuRequested( const QPoint& pos )
{
    QMenu* menu = builder()->contextMenu( "menuContext" );
    if ( menu )
        menu->popup( m_edit->mapToGlobal( pos ) );
}

void TextView::setFullScreen( bool on )
{
    QLayout* layout = mainWidget()->layout()->itemAt( 0 )->layout();
    if ( on ) {
        layout->setContentsMargins( 0, 0, 0, 0 );
        if ( QFrame* frame = qobject_cast<QFrame*>( layout->itemAt( 0 )->widget() ) )
            frame->setFrameStyle( 0 );
    } else {
        layout->setContentsMargins( 3, 0, 3, 0 );
        if ( QFrame* frame = qobject_cast<QFrame*>( layout->itemAt( 0 )->widget() ) )
            frame->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    }
}

void TextView::loadIcons()
{
    action( "lineNumbers" )->setIcon( IconLoader::icon( "line-numbers" ) );
    action( "wordWrap" )->setIcon( IconLoader::icon( "word-wrap" ) );
    action( "selectEncoding" )->setIcon( IconLoader::icon( "encoding" ) );
    action( "copy" )->setIcon( IconLoader::icon( "edit-copy" ) );
    action( "find" )->setIcon( IconLoader::icon( "find-text" ) );
    action( "findNext" )->setIcon( IconLoader::icon( "find-next" ) );
    action( "findPrevious" )->setIcon( IconLoader::icon( "find-previous" ) );
    action( "goToLine" )->setIcon( IconLoader::icon( "goto" ) );
    action( "editFile" )->setIcon( IconLoader::icon( "edit" ) );
}
