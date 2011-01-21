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

#include "settingsdialog.h"
#include "application.h"

#include "shell/shellfolder.h"
#include "utils/localsettings.h"
#include "utils/separatorcombobox.h"
#include "utils/iconloader.h"
#include "xmlui/gradientwidget.h"

SettingsDialog::SettingsDialog( QWidget* parent ) : QDialog( parent )
{
    setWindowTitle( tr( "Saladin Settings" ) );

    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    XmlUi::GradientWidget* promptWidget = new XmlUi::GradientWidget( this );
    topLayout->addWidget( promptWidget );

    QHBoxLayout* promptLayout = new QHBoxLayout( promptWidget );
    promptLayout->setSpacing( 10 );

    QLabel* promptPixmap = new QLabel( promptWidget );
    promptPixmap->setPixmap( IconLoader::pixmap( "configure", 22 ) );
    promptLayout->addWidget( promptPixmap );

    QLabel* promptLabel = new QLabel( promptWidget );
    promptLabel->setWordWrap( true );
    promptLabel->setText( tr( "Configure settings of Saladin:" ) );
    promptLabel->setMinimumWidth( 350 );
    promptLabel->setFixedHeight( promptLabel->heightForWidth( 350 ) );
    promptLayout->addWidget( promptLabel, 1 );

    QFrame* separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    topLayout->addWidget( separator );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin( 9 );
    mainLayout->setSpacing( 4 );
    topLayout->addLayout( mainLayout );

    m_tabWidget = new QTabWidget( this );
    mainLayout->addWidget( m_tabWidget );

    QWidget* generalTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( generalTab, IconLoader::icon( "configure" ), tr( "General" ) );

    QVBoxLayout* generalLayout = new QVBoxLayout( generalTab );

    QGroupBox* regionalGroup = new QGroupBox( tr( "Regional Options" ), generalTab );
    QGridLayout* regionalLayout = new QGridLayout( regionalGroup );
    generalLayout->addWidget( regionalGroup );

    QLabel* languageLabel = new QLabel( tr( "&Language of the user interface:" ), regionalGroup );
    regionalLayout->addWidget( languageLabel, 0, 0 );

    m_languageComboBox = new SeparatorComboBox( regionalGroup );
    loadLanguages();
    regionalLayout->addWidget( m_languageComboBox, 0, 1 );

    languageLabel->setBuddy( m_languageComboBox );

    regionalLayout->setColumnStretch( 2, 1 );

    QGroupBox* directoriesGroup = new QGroupBox( tr( "Initial Directories" ), generalTab );
    QGridLayout* directoriesLayout = new QGridLayout( directoriesGroup );
    generalLayout->addWidget( directoriesGroup );

    QLabel* leftPaneLabel = new QLabel( tr( "Left pane:" ), directoriesGroup );
    directoriesLayout->addWidget( leftPaneLabel, 0, 0 );

    m_leftPaneEdit = new QLineEdit( directoriesGroup );
    m_leftPaneEdit->setReadOnly( true );
    directoriesLayout->addWidget( m_leftPaneEdit, 0, 1 );

    leftPaneLabel->setBuddy( m_leftPaneEdit );

    QToolButton* leftPaneButton = browseButton( directoriesGroup, SLOT( browseLeftPane() ) );
    directoriesLayout->addWidget( leftPaneButton, 0, 2 );

    QLabel* rightPaneLabel = new QLabel( tr( "Right pane:" ), directoriesGroup );
    directoriesLayout->addWidget( rightPaneLabel, 1, 0 );

    m_rightPaneEdit = new QLineEdit( directoriesGroup );
    m_rightPaneEdit->setReadOnly( true );
    directoriesLayout->addWidget( m_rightPaneEdit, 1, 1 );

    rightPaneLabel->setBuddy( m_rightPaneEdit );

    QToolButton* rightPaneButton = browseButton( directoriesGroup, SLOT( browseRightPane() ) );
    directoriesLayout->addWidget( rightPaneButton, 1, 2 );

    m_rememberCheckBox = new QCheckBox( tr( "&Remember last directories on exit" ), directoriesGroup );
    directoriesLayout->addWidget( m_rememberCheckBox, 2, 1 );

    connect( m_rememberCheckBox, SIGNAL( toggled( bool ) ), rightPaneButton, SLOT( setDisabled( bool ) ) );
    connect( m_rememberCheckBox, SIGNAL( toggled( bool ) ), m_rightPaneEdit, SLOT( setDisabled( bool ) ) );
    connect( m_rememberCheckBox, SIGNAL( toggled( bool ) ), leftPaneButton, SLOT( setDisabled( bool ) ) );
    connect( m_rememberCheckBox, SIGNAL( toggled( bool ) ), m_leftPaneEdit, SLOT( setDisabled( bool ) ) );

    QGroupBox* updateGroup = new QGroupBox( tr( "Automatic Update" ), generalTab );
    QGridLayout* updateLayout = new QGridLayout( updateGroup );
    generalLayout->addWidget( updateGroup );

    m_updateCheckBox = new QCheckBox( tr( "&Enable automatic checking for latest version of Saladin" ), directoriesGroup );
    updateLayout->addWidget( m_updateCheckBox, 0, 0 );

    generalLayout->addStretch( 1 );

    QWidget* toolsTab = new QWidget( m_tabWidget );
    m_tabWidget->addTab( toolsTab, IconLoader::icon( "gear" ), tr( "Tools" ) );

    QVBoxLayout* toolsLayout = new QVBoxLayout( toolsTab );

    QGroupBox* viewerGroup = new QGroupBox( tr( "File Viewer" ), toolsTab );
    QGridLayout* viewerLayout = new QGridLayout( viewerGroup );
    toolsLayout->addWidget( viewerGroup );

    m_viewerEdit = new QLineEdit( viewerGroup );
    m_viewerEdit->setReadOnly( true );
    viewerLayout->addWidget( m_viewerEdit, 0, 0 );

    QToolButton* viewerButton = browseButton( viewerGroup, SLOT( browseViewer() ) );
    viewerLayout->addWidget( viewerButton, 0, 1 );

    QGroupBox* editorGroup = new QGroupBox( tr( "Text Editor" ), toolsTab );
    QGridLayout* editorLayout = new QGridLayout( editorGroup );
    toolsLayout->addWidget( editorGroup );

    m_editorEdit = new QLineEdit( editorGroup );
    m_editorEdit->setReadOnly( true );
    editorLayout->addWidget( m_editorEdit, 0, 0 );

    QToolButton* editorButton = browseButton( editorGroup, SLOT( browseEditor() ) );
    editorLayout->addWidget( editorButton, 0, 1 );

    QGroupBox* consoleGroup = new QGroupBox( tr( "Console" ), toolsTab );
    QGridLayout* consoleLayout = new QGridLayout( consoleGroup );
    toolsLayout->addWidget( consoleGroup );

    m_consoleEdit = new QLineEdit( consoleGroup );
    m_consoleEdit->setReadOnly( true );
    consoleLayout->addWidget( m_consoleEdit, 0, 0 );

    QToolButton* consoleButton = browseButton( consoleGroup, SLOT( browseConsole() ) );
    consoleLayout->addWidget( consoleButton, 0, 1 );

    QGroupBox* diffGroup = new QGroupBox( tr( "Compare Files" ), toolsTab );
    QGridLayout* diffLayout = new QGridLayout( diffGroup );
    toolsLayout->addWidget( diffGroup );

    m_diffEdit = new QLineEdit( diffGroup );
    m_diffEdit->setReadOnly( true );
    diffLayout->addWidget( m_diffEdit, 0, 0 );

    QToolButton* diffButton = browseButton( diffGroup, SLOT( browseDiff() ) );
    diffLayout->addWidget( diffButton, 0, 1 );

    toolsLayout->addStretch( 1 );

    mainLayout->addSpacing( 7 );

    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal, this );
    mainLayout->addWidget( buttonBox );

    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );
    buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&Cancel" ) );
    buttonBox->button( QDialogButtonBox::Apply )->setText( tr( "&Apply" ) );

    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
    connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

    LocalSettings* settings = application->applicationSettings();

    int index = m_languageComboBox->findData( settings->value( "Language" ).toString() );
    if ( index >= 2 )
        m_languageComboBox->setCurrentIndex( index );

    m_pidls[ 0 ] = settings->value( "Directory1" ).value<ShellPidl>();
    m_leftPaneEdit->setText( m_pidls[ 0 ].path() );

    m_pidls[ 1 ] = settings->value( "Directory2" ).value<ShellPidl>();
    m_rightPaneEdit->setText( m_pidls[ 1 ].path() );

    m_rememberCheckBox->setChecked( settings->value( "RememberDirectories" ).toBool() );

    m_updateCheckBox->setChecked( settings->value( "AutoUpdate" ).toBool() );

    m_viewerEdit->setText( settings->value( "ViewerTool" ).toString() );
    m_editorEdit->setText( settings->value( "EditorTool" ).toString() );
    m_consoleEdit->setText( settings->value( "ConsoleTool" ).toString() );
    m_diffEdit->setText( settings->value( "DiffTool" ).toString() );

    resize( 450, 300 );
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::accept()
{
    if ( apply() )
        QDialog::accept();
}

bool SettingsDialog::apply()
{
    LocalSettings* settings = application->applicationSettings();

    QString language = m_languageComboBox->itemData( m_languageComboBox->currentIndex() ).toString();
    if ( language != settings->value( "Language" ).toString() )
        QMessageBox::warning( this, tr( "Warning" ), tr( "Language settings will be applied when the application is restarted." ) );
    settings->setValue( "Language", language );

    settings->setValue( "Directory1", QVariant::fromValue( m_pidls[ 0 ] ) );
    settings->setValue( "Directory2", QVariant::fromValue( m_pidls[ 1 ] ) );
    settings->setValue( "RememberDirectories", m_rememberCheckBox->isChecked() );
    settings->setValue( "AutoUpdate", m_updateCheckBox->isChecked() );

    settings->setValue( "ViewerTool", m_viewerEdit->text() );
    settings->setValue( "EditorTool", m_editorEdit->text() );
    settings->setValue( "ConsoleTool", m_consoleEdit->text() );
    settings->setValue( "DiffTool", m_diffEdit->text() );

    settings->save();

    return true;
}

void SettingsDialog::browseLeftPane()
{
    browsePane( m_leftPaneEdit, 0 );
}

void SettingsDialog::browseRightPane()
{
    browsePane( m_rightPaneEdit, 1 );
}

void SettingsDialog::browsePane( QLineEdit* edit, int index )
{
    ShellPidl result = ShellFolder::browseFolder( this, tr( "Select initial directory:" ), m_pidls[ index ] );
    if ( result.isValid() ) {
        m_pidls[ index ] = result;
        edit->setText( result.path() );
    }
}

void SettingsDialog::browseViewer()
{
    browseTool( m_viewerEdit );
}

void SettingsDialog::browseEditor()
{
    browseTool( m_editorEdit );
}

void SettingsDialog::browseConsole()
{
    browseTool( m_consoleEdit );
}

void SettingsDialog::browseDiff()
{
    browseTool( m_diffEdit );
}

void SettingsDialog::browseTool( QLineEdit* edit )
{
    QString filter = tr( "Applications" ) + " (*.exe);;" + tr( "All Files" ) +  " (*.*)";

    QString result = QFileDialog::getOpenFileName( this, tr( "Select Tool" ), edit->text(), filter );
    if ( !result.isEmpty() )
        edit->setText( QDir::toNativeSeparators( result ) );
}

void SettingsDialog::loadLanguages()
{
    m_languageComboBox->addItem( tr( "System Default" ) );
    m_languageComboBox->addSeparator();

    QSettings settings( application->translationsPath() + "/locale.ini", QSettings::IniFormat );
    settings.setIniCodec( "UTF8" );

    settings.beginGroup( "languages" );
    QStringList languages = settings.allKeys();

    if ( languages.isEmpty() )
        m_languageComboBox->addItem( "English / United States", "en_US" );

    foreach ( QString language, languages ) {
        QString name = settings.value( language ).toString();
        m_languageComboBox->addItem( name, language );
    }

    settings.endGroup();
}

QToolButton* SettingsDialog::browseButton( QWidget* parent, const char* slot )
{
    QToolButton* button = new QToolButton( parent );
    button->setAutoRaise( true );
    button->setIcon( IconLoader::icon( "browse" ) );
    button->setIconSize( QSize( 16, 16 ) );
    button->setToolTip( tr( "Browse" ) );

    connect( button, SIGNAL( clicked() ), this, slot );

    return button;
}
