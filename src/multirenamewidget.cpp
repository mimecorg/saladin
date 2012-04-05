/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2012 Michał Męciński
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

#include "multirenamewidget.h"

MultiRenameWidget::MultiRenameWidget( QWidget* parent ) : QWidget( parent )
{
    QGridLayout* layout = new QGridLayout( this );
    layout->setMargin( 0 );

    layout->setColumnMinimumWidth( 2, 5 );

    QLabel* nameLabel = new QLabel( tr( "&Name:" ), this );
    layout->addWidget( nameLabel, 0, 0 );

    m_nameEdit = new QLineEdit( "[n]", this );
    layout->addWidget( m_nameEdit, 0, 1 );

    nameLabel->setBuddy( m_nameEdit );

    QLabel* extensionLabel = new QLabel( tr( "&Extension:" ), this );
    layout->addWidget( extensionLabel, 0, 3 );

    m_extensionEdit = new QLineEdit( "[e]", this );
    layout->addWidget( m_extensionEdit, 0, 4 );

    extensionLabel->setBuddy( m_extensionEdit );

    QLabel* searchLabel = new QLabel( tr( "&Search:" ), this );
    layout->addWidget( searchLabel, 1, 0 );

    m_searchEdit = new QLineEdit( this );
    layout->addWidget( m_searchEdit, 1, 1 );

    searchLabel->setBuddy( m_searchEdit );

    QLabel* replaceLabel = new QLabel( tr( "&Replace:" ), this );
    layout->addWidget( replaceLabel, 1, 3 );

    m_replaceEdit = new QLineEdit( this );
    layout->addWidget( m_replaceEdit, 1, 4 );

    replaceLabel->setBuddy( m_replaceEdit );

    QHBoxLayout* checkBoxLayout = new QHBoxLayout();
    layout->addLayout( checkBoxLayout, 2, 1 );

    m_regExpCheckBox = new QCheckBox( tr( "&Regular expressions" ), this );
    checkBoxLayout->addWidget( m_regExpCheckBox );

    m_caseCheckBox = new QCheckBox( tr( "Case sensi&tive" ), this );
    checkBoxLayout->addWidget( m_caseCheckBox );

    checkBoxLayout->addStretch( 1 );

    QLabel* caseLabel = new QLabel( tr( "C&ase:" ), this );
    layout->addWidget( caseLabel, 2, 3 );

    m_caseComboBox = new QComboBox( this );
    m_caseComboBox->addItem( tr( "Unchanged" ) );
    m_caseComboBox->addItem( tr( "Lower (file name.ext)" ) );
    m_caseComboBox->addItem( tr( "Upper (FILE NAME.EXT)" ) );
    m_caseComboBox->addItem( tr( "Mixed (FILE NAME.ext)" ) );
    m_caseComboBox->addItem( tr( "Sentence (File name.ext)" ) );
    m_caseComboBox->addItem( tr( "Title (File Name.ext)" ) );
    layout->addWidget( m_caseComboBox, 2, 4 );

    caseLabel->setBuddy( m_caseComboBox );

    QLabel* previewLabel = new QLabel( tr( "Preview:" ), this );
    layout->addWidget( previewLabel, 3, 0, 1, 5 );

    m_list = new QTreeWidget( this );
    m_list->setSelectionMode( QAbstractItemView::SingleSelection );
    m_list->setRootIsDecorated( false );
    m_list->setUniformRowHeights( true );
    m_list->header()->setMovable( false );
    m_list->header()->setStretchLastSection( false );
    layout->addWidget( m_list, 4, 0, 1, 5 );

    QTreeWidgetItem* header = new QTreeWidgetItem();
    header->setText( 0, tr( "Old Name" ) );
    header->setText( 1, tr( "New Name" ) );
    m_list->setHeaderItem( header );

    m_list->setColumnWidth( 0, 320 );
    m_list->setColumnWidth( 1, 320 );

    connect( m_nameEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( transformNames() ) );
    connect( m_extensionEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( transformNames() ) );
    connect( m_searchEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( transformNames() ) );
    connect( m_replaceEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( transformNames() ) );
    connect( m_regExpCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( transformNames() ) );
    connect( m_caseCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( transformNames() ) );
    connect( m_caseComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( transformNames() ) );

    m_nameEdit->selectAll();

    setFocusProxy( m_nameEdit );
}

MultiRenameWidget::~MultiRenameWidget()
{
}

void MultiRenameWidget::setInputNames( const QStringList& names )
{
    m_inputNames = names;

    m_list->clear();

    foreach ( QString name, names ) {
        QTreeWidgetItem* item = new QTreeWidgetItem( m_list );
        item->setText( 0, name );
    }

    transformNames();
}

static int parseInt( const QChar* data, const QChar** endData, int defaultValue )
{
    if ( *data == QLatin1Char( ':' ) || *data == QLatin1Char( ']' ) ) {
        *endData = data;
        return defaultValue;
    }

    bool negative = false;

    if ( *data == QLatin1Char( '-' ) ) {
        negative = true;
        data++;
    }

    bool empty = true;
    int result = 0;

    while ( data->isDigit() ) {
        result = result * 10 + data->digitValue();
        empty = false;
        data++;
        if ( result < 0 ) {
            *endData = NULL;
            return 0;
        }
    }

    *endData = !empty ? data : NULL;

    return negative ? -result : result;
}

static QStringList transformMask( const QString& mask, const QStringList& names, const QStringList& extensions )
{
    QStringList result;

    int count = names.count();
    for ( int i = 0; i < count; i++ )
        result.append( QString() );

    const QChar* data = mask.constData();

    for ( QChar ch = *data++; ch != 0; ch = *data++ ) {
        if ( ch == QLatin1Char( '[' ) ) {
            const QChar* opdata = data;
            QChar op = ( *opdata++ ).toLower();
            if ( op == QLatin1Char( 'n' ) || op == QLatin1Char( 'e' ) ) {
                int from = 1;
                int to = -1;
                if ( *opdata == ':' )
                    from = to = parseInt( opdata + 1, &opdata, 1 );
                if ( opdata && *opdata == ':' )
                    to = parseInt( opdata + 1, &opdata, -1 );
                if ( opdata && *opdata == ']' ) {
                    for ( int i = 0; i < count; i++ ) {
                        QString part = ( op == QLatin1Char( 'n' ) ) ? names.at( i ) : extensions.at( i );
                        int length = part.length();
                        int pos = ( from > 0 ) ? qMin( from - 1, length ) : qMax( 0, length + from );
                        int n = ( to > 0 ) ? qMin( to - pos, length - pos ) : qMax( 0, length + to - pos + 1 );
                        if ( pos < length && n > 0 )
                            result[ i ].append( part.mid( pos, n ) );
                    }
                    data = opdata + 1;
                    continue;
                }
            }
            if ( op == QLatin1Char( 'c' ) ) {
                int width = 1;
                int from = 1;
                int step = 1;
                if ( *opdata == ':' )
                    width = parseInt( opdata + 1, &opdata, 1 );
                if ( opdata && *opdata == ':' )
                    from = parseInt( opdata + 1, &opdata, 1 );
                if ( opdata && *opdata == ':' )
                    step = parseInt( opdata + 1, &opdata, 1 );
                if ( opdata && *opdata == ']' ) {
                    for ( int i = 0; i < count; i++ ) {
                        int number = from + i * step;
                        result[ i ].append( QString( "%1" ).arg( number, qBound( 1, width, 10 ), 10, QLatin1Char( '0' ) ) );
                    }
                    data = opdata + 1;
                    continue;
                }
            }
        }
        for ( int i = 0; i < count; i++ )
            result[ i ].append( ch );
    }

    return result;
}

static QString toTitleCase( const QString& input )
{
    QString output = input;

    bool start = true;

    for ( QChar* data = output.data(); *data != 0; data++ ) {
        if ( data->isLetter() ) {
            *data = start ? data->toUpper() : data->toLower();
            start = false;
        } else {
            start = data->isSpace();
        }
    }

    return output;
}

void MultiRenameWidget::transformNames()
{
    QStringList names;
    QStringList extensions;

    int count = m_inputNames.count();

    for ( int i = 0; i < count; i++ ) {
        QString name = m_inputNames.at( i );
        QString extension;
        int pos = name.lastIndexOf( '.' );
        if ( pos > 0 ) {
            extension = name.mid( pos + 1 );
            name.truncate( pos );
        }
        names.append( name );
        extensions.append( extension );
    }

    QStringList newNames = transformMask( m_nameEdit->text(), names, extensions );
    QStringList newExtensions = transformMask( m_extensionEdit->text(), names, extensions );

    QString search = m_searchEdit->text();
    if ( !search.isEmpty() ) {
        QRegExp regExp;

        Qt::CaseSensitivity sensitivity = m_caseCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;

        bool isRegExp = m_regExpCheckBox->isChecked();
        if ( isRegExp ) {
            regExp.setCaseSensitivity( sensitivity );
            regExp.setPatternSyntax( QRegExp::RegExp2 );
            regExp.setPattern( search );
        }

        if ( !isRegExp || regExp.isValid() ) {
            QString replace = m_replaceEdit->text();

            for ( int i = 0; i < count; i++ ) {
                QString name = newNames.at( i );
                if ( !newExtensions.at( i ).isEmpty() ) {
                    name += QLatin1Char( '.' );
                    name += newExtensions.at( i );
                }

                if ( isRegExp )
                    name = name.replace( regExp, replace );
                else
                    name = name.replace( search, replace, sensitivity );

                QString extension;
                int pos = name.lastIndexOf( '.' );
                if ( pos > 0 ) {
                    extension = name.mid( pos + 1 );
                    name.truncate( pos );
                }
                newNames[ i ] = name;
                newExtensions[ i ] = extension;
            }
        }
    }

    for ( int i = 0; i < count; i++ ) {
        switch ( m_caseComboBox->currentIndex() ) {
            case 1:
                newNames[ i ] = newNames[ i ].toLower();
                break;
            case 2:
            case 3:
                newNames[ i ] = newNames[ i ].toUpper();
                break;
            case 4:
                newNames[ i ] = newNames[ i ].left( 1 ).toUpper() + newNames[ i ].mid( 1 ).toLower();
                break;
            case 5:
                newNames[ i ] = toTitleCase( newNames[ i ] );
                break;
        }

        switch ( m_caseComboBox->currentIndex() ) {
            case 1:
            case 3:
            case 4:
            case 5:
                newExtensions[ i ] = newExtensions[ i ].toLower();
                break;
            case 2:
                newExtensions[ i ] = newExtensions[ i ].toUpper();
                break;
        }
    }

    m_outputNames.clear();
    for ( int i = 0; i < count; i++ ) {
        QString name = newNames.at( i );
        if ( !newExtensions.at( i ).isEmpty() ) {
            name += QLatin1Char( '.' );
            name += newExtensions.at( i );
        }
        m_outputNames.append( name );
        m_list->topLevelItem( i )->setText( 1, name );
    }
}
