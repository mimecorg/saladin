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

#include "utils/iconloader.h"
#include "xmlui/toolstrip.h"

TextView::TextView( QObject* parent, QWidget* parentWidget ) : View( parent )
{
    QAction* action;
    XmlUi::ToolStripAction* encodingAction;

    action = new QAction( IconLoader::icon( "word-wrap" ), tr( "Word Wrap" ), this );
    action->setShortcut( Qt::Key_W );
    action->setCheckable( true );
    action->setChecked( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( toggleWordWrap() ) );
    setAction( "wordWrap", action );

    encodingAction = new XmlUi::ToolStripAction( IconLoader::icon( "encoding" ), tr( "Encoding" ), this );
    encodingAction->setPopupMode( QToolButton::InstantPopup );
    connect( encodingAction, SIGNAL( triggered() ), this, SLOT( selectEncoding() ) );
    setAction( "selectEncoding", encodingAction );

    loadXmlUiFile( ":/resources/textview.xml" );

    m_edit = new QPlainTextEdit( parentWidget );
    m_edit->setReadOnly( true );

    m_edit->setFont( QFont( "Courier New", 10 ) );

    setMainWidget( m_edit );

    m_encodingMapper = new QSignalMapper( this );
    connect( m_encodingMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( setEncoding( const QString& ) ) );

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
        action = new QAction( tr( stdEncodings[ i ].m_name ), this );
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

        action = new QAction( text, this );
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

    encodingAction->setMenu( menu );
}

TextView::~TextView()
{
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
