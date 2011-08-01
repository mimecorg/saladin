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

#include "textview.h"

#include "application.h"
#include "findbar.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "xmlui/toolstrip.h"

TextView::TextView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_isFindEnabled( false ),
    m_isFindInitialized( false )
{
    QAction* action;
    XmlUi::ToolStripAction* encodingAction;

    action = new QAction( IconLoader::icon( "word-wrap" ), tr( "Word Wrap" ), this );
    action->setShortcut( Qt::Key_W );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( toggleWordWrap() ) );
    setAction( "wordWrap", action );

    encodingAction = new XmlUi::ToolStripAction( IconLoader::icon( "encoding" ), tr( "Encoding" ), this );
    encodingAction->setPopupMode( QToolButton::InstantPopup );
    connect( encodingAction, SIGNAL( triggered() ), this, SLOT( selectEncoding() ) );
    setAction( "selectEncoding", encodingAction );

    action = new QAction( IconLoader::icon( "find-text" ), tr( "&Find..." ), this );
    action->setShortcut( QKeySequence::Find );
    connect( action, SIGNAL( triggered() ), this, SLOT( find() ) );
    setAction( "find", action );

    action = new QAction( IconLoader::icon( "find-next" ), tr( "Find &Next" ), this );
    action->setShortcut( QKeySequence::FindNext );
    connect( action, SIGNAL( triggered() ), this, SLOT( findNext() ) );
    setAction( "findNext", action );

    action = new QAction( IconLoader::icon( "find-previous" ), tr( "Find &Previous" ), this );
    action->setShortcut( QKeySequence::FindPrevious );
    connect( action, SIGNAL( triggered() ), this, SLOT( findPrevious() ) );
    setAction( "findPrevious", action );

    loadXmlUiFile( ":/resources/textview.xml" );

    QWidget* main = new QWidget( parentWidget );
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );

    m_edit = new QPlainTextEdit( main );
    m_edit->setReadOnly( true );

    m_edit->setFont( QFont( "Courier New", 10 ) );

    QPalette palette = m_edit->palette();
    palette.setBrush( QPalette::Inactive, QPalette::Highlight, palette.brush( QPalette::Active, QPalette::Highlight ) );
    palette.setBrush( QPalette::Inactive, QPalette::HighlightedText, palette.brush( QPalette::Active, QPalette::HighlightedText ) );
    m_edit->setPalette( palette );

    mainLayout->addWidget( m_edit );

    m_findBar = new FindBar( main );
    m_findBar->setBoundWidget( m_edit );
    m_findBar->setAutoFillBackground( true );
    m_findBar->hide();

    mainLayout->addWidget( m_findBar );

    connect( m_findBar, SIGNAL( find( const QString& ) ), this, SLOT( findText( const QString& ) ) );
    connect( m_findBar, SIGNAL( findPrevious() ), this, SLOT( findPrevious() ) );
    connect( m_findBar, SIGNAL( findNext() ), this, SLOT( findNext() ) );
    connect( m_findBar, SIGNAL( findEnabled( bool ) ), this, SLOT( updateActions() ) );

    setMainWidget( main );

    main->installEventFilter( this );

    m_encodingMapper = new QSignalMapper( this );
    connect( m_encodingMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( setEncoding( const QString& ) ) );

    encodingAction->setMenu( createEncodingMenu() );

    initializeSettings();
}

TextView::~TextView()
{
    storeSettings();
}

void TextView::initializeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    bool wordWrap = settings->value( "WordWrap", true ).toBool();
    m_edit->setWordWrapMode( wordWrap ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap );
    action( "wordWrap" )->setChecked( wordWrap );

    updateActions();
}

void TextView::storeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "WordWrap", action( "wordWrap" )->isChecked() );
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

void TextView::load( const QString& path, const QByteArray& format )
{
    m_path = path;
    m_format = format;

    reload();
}

void TextView::reload()
{
    QFile file( m_path );

    if ( file.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &file );
        stream.setAutoDetectUnicode( false );
        stream.setCodec( m_format );

        m_edit->setPlainText( stream.readAll() );
    }

    QObject* current = m_encodingMapper->mapping( QString( m_format ) );

    QList<QAction*> actions = action( "selectEncoding" )->menu()->actions();
    foreach ( QAction* action, actions ) {
        if ( action->isCheckable() )
            action->setChecked( action == current );
    }
}

void TextView::toggleWordWrap()
{
    m_edit->setWordWrapMode( action( "wordWrap" )->isChecked() ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap );
}

void TextView::setEncoding( const QString& format )
{
    m_format = format.toLatin1();

    reload();
}

void TextView::updateActions()
{
    m_isFindEnabled = m_findBar->isFindEnabled();

    action( "findNext" )->setEnabled( m_isFindEnabled );
    action( "findPrevious" )->setEnabled( m_isFindEnabled );
}

void TextView::find()
{
    if ( !m_isFindInitialized ) {
        LocalSettings* settings = application->applicationSettings();
        QString text = settings->value( "FindText" ).toString();
        QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings->value( "FindFlags" ).toInt();
        m_findBar->setText( text );
        m_findBar->setFlags( flags );
        m_isFindInitialized = true;
    }

    m_findBar->show();
    m_findBar->setFocus();
}

void TextView::findText( const QString& text )
{
    LocalSettings* settings = application->applicationSettings();
    settings->setValue( "FindText", text );
    settings->setValue( "FindFlags", (int)m_findBar->flags() );

    findText( text, m_edit->textCursor().selectionStart(), m_findBar->flags() );
}

void TextView::findNext()
{
    if ( m_isFindEnabled )
        findText( m_findBar->text(), m_edit->textCursor().selectionStart() + 1, m_findBar->flags() );
}

void TextView::findPrevious()
{
    if ( m_isFindEnabled )
        findText( m_findBar->text(), m_edit->textCursor().selectionStart(), m_findBar->flags() | QTextDocument::FindBackward );
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

    if ( found.isNull() ) {
        found = m_edit->textCursor();
        found.setPosition( found.selectionStart() );
    }

    m_findBar->show();
    m_findBar->setFocus();

    m_edit->setTextCursor( found );

    m_findBar->showWarning( warn );
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
