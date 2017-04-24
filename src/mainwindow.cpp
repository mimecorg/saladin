/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2017 Michał Męciński
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

#include "mainwindow.h"
#include "application.h"
#include "panewidget.h"
#include "drivestripmanager.h"
#include "operationdialog.h"
#include "openftpdialog.h"
#include "bookmarksdialog.h"
#include "searchdialog.h"
#include "settingsdialog.h"

#include "shell/shellfolder.h"
#include "shell/shellselection.h"
#include "shell/shelldropdata.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "viewer/viewmanager.h"
#include "xmlui/toolstrip.h"
#include "xmlui/builder.h"

MainWindow* mainWindow = NULL;

MainWindow::MainWindow() : QMainWindow(),
    m_driveStripManager( NULL ),
    m_viewManager( NULL ),
    m_sourcePane( NULL ),
    m_targetPane( NULL )
{
    mainWindow = this;

    setWindowTitle( tr( "Saladin" ) );

    QAction* action;

    action = new QAction( tr( "About Saladin" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F1 ) );
    connect( action, SIGNAL( triggered() ), application, SLOT( about() ) );
    setAction( "about", action );

    action = new QAction( tr( "Release Notes" ), this );
    connect( action, SIGNAL( triggered() ), application, SLOT( openWebsite() ) );
    setAction( "releaseNotes", action );

    action = new QAction( tr( "Quick Quide" ), this );
    connect( action, SIGNAL( triggered() ), application, SLOT( showQuickGuide() ) );
    setAction( "showQuickGuide", action );

    action = new QAction( tr( "Settings..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( configure() ) );
    setAction( "configure", action );

    action = new QAction( tr( "Paste" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_V ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( paste() ) );
    setAction( "paste", action );

    action = new QAction( tr( "Cut" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_X ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( cut() ) );
    setAction( "cut", action );

    action = new QAction( tr( "Copy" ), this );
    setAction( "popupEditCopy", action );

    action = new QAction( tr( "Copy Files" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_C ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( copy() ) );
    setAction( "copy", action );

    action = new QAction( tr( "Copy File Names" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyNames() ) );
    setAction( "copyNames", action );

    action = new QAction( tr( "Refresh" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( refresh() ) );
    setAction( "refresh", action );

    action = new QAction( tr( "Refresh Drives" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( refreshDrives() ) );
    setAction( "refreshDrives", action );

    action = new QAction( tr( "View Hidden Files" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_H ) );
    action->setIconText( tr( "Hidden" ) );
    action->setCheckable( true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( viewHidden( bool ) ) );
    setAction( "viewHidden", action );

    action = new QAction( tr( "Same Directory" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_E ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( viewSameDirectory() ) );
    setAction( "viewSameDirectory", action );

    action = new QAction( tr( "Swap Panels" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_U ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( swapPanes() ) );
    setAction( "swapPanes", action );

    action = new QAction( tr( "Home Directory" ), this );
    action->setShortcut( QKeySequence( Qt::ALT + Qt::Key_Home ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openHome() ) );
    setAction( "openHome", action );

    action = new QAction( tr( "Root Directory" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Backslash ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openRoot() ) );
    setAction( "openRoot", action );

    action = new QAction( tr( "Parent Directory" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( openParent() ) );
    setAction( "openParent", action );

    action = new QAction( tr( "Left -> Right" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyToRightPane() ) );
    setAction( "copyToRightPane", action );

    action = new QAction( tr( "Left <- Right" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyToLeftPane() ) );
    setAction( "copyToLeftPane", action );

    action = new QAction( tr( "Select Mask..." ), this );
    action->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::CTRL + Qt::Key_Plus ) << QKeySequence( Qt::CTRL + Qt::Key_Equal ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( selectMask() ) );
    setAction( "selectMask", action );

    action = new QAction( tr( "Select All" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_A ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
    setAction( "selectAll", action );

    action = new QAction( tr( "Unselect Mask..." ), this );
    action->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::CTRL + Qt::Key_Minus ) << QKeySequence( Qt::CTRL + Qt::Key_Underscore ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( unselectMask() ) );
    setAction( "unselectMask", action );

    action = new QAction( tr( "Unselect All" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_A ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( unselectAll() ) );
    setAction( "unselectAll", action );

    action = new QAction( tr( "Invert Selection" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_I ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( invertSelection() ) );
    setAction( "invertSelection", action );

    action = new QAction( tr( "Rename" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F2 ) );
    action->setIconText( tr( "Rename\nF2" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( renameCurrent() ) );
    setAction( "renameCurrent", action );

    action = new QAction( tr( "View", "action name" ), this );
    action->setIconText( tr( "View\nF3" ) );
    setAction( "popupView", action );

    action = new QAction( tr( "View Current File" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F3 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( viewCurrent() ) );
    setAction( "viewCurrent", action );

    action = new QAction( tr( "View Selected Files" ), this );
    action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_F3 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( viewSelected() ) );
    setAction( "viewSelected", action );

    action = new QAction( tr( "Edit" ), this );
    action->setIconText( tr( "Edit\nF4" ) );
    setAction( "popupEdit", action );

    action = new QAction( tr( "Edit File" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F4 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editCurrent() ) );
    setAction( "editCurrent", action );

    action = new QAction( tr( "Create File..." ), this );
    action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_F4 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editNew() ) );
    setAction( "editNew", action );

    action = new QAction( tr( "Copy" ), this );
    action->setIconText( tr( "Copy\nF5" ) );
    setAction( "popupCopy", action );

    action = new QAction( tr( "Copy To Target Directory..." ), this );
    action->setShortcut( QKeySequence( Qt::Key_F5 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( copySelected() ) );
    setAction( "copySelected", action );

    action = new QAction( tr( "Clone In Source Directory..." ), this );
    action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_F5 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( cloneSelected() ) );
    setAction( "cloneSelected", action );

    action = new QAction( tr( "Move" ), this );
    action->setIconText( tr( "Move\nF6" ) );
    setAction( "popupMove", action );

    action = new QAction( tr( "Move To Target Directory..." ), this );
    action->setShortcut( QKeySequence( Qt::Key_F6 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( moveSelected() ) );
    setAction( "moveSelected", action );

    action = new QAction( tr( "Rename In Source Directory..." ), this );
    action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_F6 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( renameSelected() ) );
    setAction( "renameSelected", action );

    action = new QAction( tr( "Create Folder..." ), this );
    action->setShortcut( QKeySequence( Qt::Key_F7 ) );
    action->setIconText( tr( "Folder\nF7" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( createFolder() ) );
    setAction( "createFolder", action );

    action = new QAction( tr( "Delete" ), this );
    action->setIconText( tr( "Delete\nF8" ) );
    setAction( "popupDelete", action );

    action = new QAction( tr( "Move To Recycle Bin" ), this );
    action->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::Key_F8 ) << QKeySequence( Qt::Key_Delete ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( moveToTrashCan() ) );
    setAction( "moveToTrashCan", action );

    action = new QAction( tr( "Delete Permanently" ), this );
    action->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::SHIFT + Qt::Key_F8 ) << QKeySequence( Qt::SHIFT + Qt::Key_Delete ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( deleteSelected() ) );
    setAction( "deleteSelected", action );

    action = new QAction( tr( "Open Console" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F9 ) );
    action->setIconText( tr( "Console\nF9" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openTerminal() ) );
    setAction( "openTerminal", action );

    action = new QAction( tr( "Pack..." ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_P ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( packToZip() ) );
    setAction( "packToZip", action );

    action = new QAction( tr( "Open" ), this );
    setAction( "popupOpen", action );

    action = new QAction( tr( "Open Directory" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_O ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openDirectory() ) );
    setAction( "openDirectory", action );

    action = new QAction( tr( "Browse For Folder..." ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_O ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( browse() ) );
    setAction( "browse", action );

    action = new QAction( tr( "Connect To FTP..." ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openFtpSite() ) );
    setAction( "openFtpSite", action );

    action = new QAction( tr( "Calculate Size" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_L ) );
    action->setIconText( tr( "Size" ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( calculateSize() ) );
    setAction( "calculateSize", action );

    action = new QAction( tr( "Compare" ), this );
    setAction( "popupCompare", action );

    action = new QAction( tr( "Compare Files" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( compareFiles() ) );
    setAction( "compareFiles", action );

    action = new QAction( tr( "Compare Directories" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_M ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( compareDirectories() ) );
    setAction( "compareDirectories", action );

    action = new QAction( tr( "Search..." ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( search() ) );
    setAction( "search", action );

    action = new QAction( tr( "Explore" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_E ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( explore() ) );
    setAction( "explore", action );

    action = new QAction( tr( "History" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( showHistory() ) );
    setAction( "showHistory", action );

    action = new QAction( tr( "Bookmarks" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_D ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( showBookmarks() ) );
    setAction( "showBookmarks", action );

    action = new QAction( tr( "Add Bookmark..." ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_D ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( addBookmark() ) );
    setAction( "addBookmark", action );

    action = new QAction( tr( "Edit Bookmarks..." ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( editBookmarks() ) );
    setAction( "editBookmarks", action );

    action = new QAction( tr( "Collapse Toolbar" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( hideToolStrip() ) );
    setAction( "hideToolStrip", action );

    action = new QAction( tr( "Expand Toolbar" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( showToolStrip() ) );
    setAction( "showToolStrip", action );

    loadIcons();

    connect( application, SIGNAL( themeChanged() ), this, SLOT( loadIcons() ) );

    setTitle( "sectionPanels", tr( "Panels" ) );
    setTitle( "sectionSelect", tr( "Select" ) );

    setPopupMenu( "popupView", "menuView", "viewCurrent" );
    setPopupMenu( "popupEdit", "menuEdit", "editCurrent" );
    setPopupMenu( "popupCopy", "menuCopy", "copySelected" );
    setPopupMenu( "popupMove", "menuMove", "moveSelected" );
    setPopupMenu( "popupDelete", "menuDelete", "moveToTrashCan" );
    setPopupMenu( "popupEditCopy", "menuEditCopy", "copy" );
    setPopupMenu( "popupOpen", "menuOpen", "browse" );
    setPopupMenu( "popupCompare", "menuCompare", "compareFiles" );

    loadXmlUiFile( ":/resources/mainwindow.xml" );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->addClient( this );

    if ( application->applicationSettings()->value( "HideToolStrip" ).toBool() )
        createMenuBar();
    else
        createToolStrip();

    QShortcut* shortcut;

    shortcut = new QShortcut( Qt::ALT + Qt::Key_F1, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( showDrivesMenu1() ) );

    shortcut = new QShortcut( Qt::ALT + Qt::Key_F2, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( showDrivesMenu2() ) );

    shortcut = new QShortcut( Qt::ALT + Qt::Key_Return, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( showProperties() ) );

    shortcut = new QShortcut( Qt::CTRL + Qt::Key_Right, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( otherOpenFolder() ) );

    shortcut = new QShortcut( Qt::CTRL + Qt::Key_Left, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( otherOpenParent() ) );

    QWidget* widget = new QWidget( this );
    setCentralWidget( widget );

    m_panes[ 0 ] = new PaneWidget( PaneWidget::LeftPane, widget );
    m_panes[ 1 ] = new PaneWidget( PaneWidget::RightPane, widget );

    QHBoxLayout* layout = new QHBoxLayout( widget );
    layout->setContentsMargins( 3, 0, 3, 0 );
    layout->setSpacing( 5 );

    layout->addWidget( m_panes[ 0 ], 1 );
    layout->addWidget( m_panes[ 1 ], 1 );

    connect( m_panes[ 0 ], SIGNAL( headerSectionResized( int, int ) ), m_panes[ 1 ], SLOT( resizeHeaderSection( int, int ) ) );
    connect( m_panes[ 1 ], SIGNAL( headerSectionResized( int, int ) ), m_panes[ 0 ], SLOT( resizeHeaderSection( int, int ) ) );
    connect( m_panes[ 0 ], SIGNAL( headerSectionMoved( int, int ) ), m_panes[ 1 ], SLOT( moveHeaderSection( int, int ) ) );
    connect( m_panes[ 1 ], SIGNAL( headerSectionMoved( int, int ) ), m_panes[ 0 ], SLOT( moveHeaderSection( int, int ) ) );

    application->installEventFilter( this );

    initializeSettings();
}

MainWindow::~MainWindow()
{
    ShellDropData::unregisterDropTarget( windowHandle() );

    saveSettings();

    delete m_viewManager;
    m_viewManager = NULL;
}

void MainWindow::initializeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    action( "viewHidden" )->setChecked( settings->value( "ViewHiddenFiles" ).toBool() );

    if ( settings->contains( "MainWindowGeometry" ) ) {
        restoreGeometry( settings->value( "MainWindowGeometry" ).toByteArray() );
    } else {
        QRect available = QApplication::desktop()->availableGeometry( this );
        resize( available.width() * 4 / 5, available.height() * 4 / 5 );
        setWindowState( Qt::WindowMaximized );
    }

    for ( int i = 0; i < 2; i++ )
        m_panes[ i ]->restoreSettings();

    show();

    for ( int i = 0; i < 2; i++ )
        m_panes[ i ]->updateEditPalette();

    QApplication::processEvents();

    QApplication::setOverrideCursor( Qt::BusyCursor );

    m_driveStripManager = new DriveStripManager( this );

    for ( int i = 0; i < 2; i++ )
        m_panes[ i ]->populateDrives();

    m_viewManager = new ViewManager();

    QApplication::processEvents();

    for ( int i = 0; i < 2; i++ ) {
        QString key;
        if ( settings->value( "RememberDirectories" ).toBool() )
            key = QString( "Directory%1" ).arg( i + 1 );
        else
            key = QString( "HomeDirectory%1" ).arg( i + 1 );
        restoreDirectory( i, key );
    }

    m_panes[ 0 ]->activateView();

    ShellDropData::registerDropTarget( windowHandle() );

    QApplication::restoreOverrideCursor();
}

void MainWindow::saveSettings()
{
    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "ViewHiddenFiles", action( "viewHidden" )->isChecked() );

    settings->setValue( "MainWindowGeometry", saveGeometry() );

    for ( int i = 0; i < 2; i++ )
        m_panes[ i ]->saveSettings();

    if ( settings->value( "RememberDirectories" ).toBool() ) {
        for ( int i = 0; i < 2; i++ ) {
            ShellFolder* folder = m_panes[ i ]->folder();
            QString path = folder->path();
            if ( !path.startsWith( QLatin1String( "ftp://" ), Qt::CaseInsensitive ) ) {
                QString key = QString( "Directory%1" ).arg( i + 1 );
                settings->setValue( key, QVariant::fromValue( folder->pidl() ) );
            }
        }
    }
}

void MainWindow::restoreDirectory( int index, const QString& key )
{
    LocalSettings* settings = application->applicationSettings();

    ShellPidl pidl = settings->value( key ).value<ShellPidl>();

    ShellFolder* folder = new ShellFolder( pidl, m_panes[ index ] );
    if ( !folder->isValid() ) {
        delete folder;
        folder = new ShellFolder( ShellFolder::defaultFolder(), m_panes[ index ] );
    }

    m_panes[ index ]->setFolder( folder );
}

void MainWindow::closeEvent( QCloseEvent* e )
{
    application->quit();
    e->ignore();
}

bool MainWindow::eventFilter( QObject* object, QEvent* e )
{
    if ( e->type() != QEvent::MouseButtonPress &&
        e->type() != QEvent::MouseButtonDblClick &&
        e->type() != QEvent::FocusIn &&
        e->type() != QEvent::ContextMenu )
        return false;

    if ( !object->isWidgetType() )
        return false;

    QWidget* widget = static_cast<QWidget*>( object );

    if ( ( ( widget->windowFlags().testFlag( Qt::Dialog ) ) && widget->isModal() ) ||
        ( widget->windowFlags().testFlag( Qt::Popup ) ) || ( widget->windowFlags().testFlag( Qt::Tool ) ) )
        return false;

    while ( widget ) {
        if ( widget->topLevelWidget() != this )
            return false;

        int index = -1;
        if ( m_panes[ 0 ] == widget )
            index = 0;
        else if ( m_panes[ 1 ] == widget )
            index = 1;

        if ( index >= 0 ) {
            setSourcePane( index );
            return false;
        }

        widget = widget->parentWidget();
    }

    return false;
}

void MainWindow::changeEvent( QEvent* e )
{
    QMainWindow::changeEvent( e );

    if ( e->type() == QEvent::ThemeChange )
        QTimer::singleShot( 100, application, SLOT( updateTheme() ) );
}

void MainWindow::setSourcePane( int index )
{
    if ( m_sourcePane == m_panes[ index ] )
        return;

    m_sourcePane = m_panes[ index ];
    m_targetPane = m_panes[ 1 - index ];

    m_sourcePane->setSourcePane( true );
    m_targetPane->setSourcePane( false );
}

void MainWindow::configure()
{
    SettingsDialog dialog( this );
    dialog.exec();
}

void MainWindow::paste()
{
    m_sourcePane->folder()->invokeCommand( "paste" );
}

void MainWindow::cut()
{
    invokeCommand( m_sourcePane->folder(), m_sourcePane->selectedItems(), "cut" );
}

void MainWindow::copy()
{
    invokeCommand( m_sourcePane->folder(), m_sourcePane->selectedItems(), "copy" );
}

void MainWindow::invokeCommand( ShellFolder* folder, const QList<ShellItem>& items, const char* verb )
{
    if ( items.isEmpty() )
        return;

    ShellSelection selection( folder, items, this );

    selection.invokeCommand( verb );
}

void MainWindow::copyNames()
{
    QList<ShellItem> items = m_sourcePane->selectedItems();
    if ( items.isEmpty() )
        return;

    QStringList names;
    foreach ( ShellItem item, items )
        names.append( item.name() );

    QString text;
    if ( names.count() > 1 )
        text = names.join( "\r\n" ) + "\r\n";
    else
        text = names.first();

    QApplication::clipboard()->setText( text );
}

void MainWindow::refresh()
{
    m_sourcePane->refresh();
}

void MainWindow::refreshDrives()
{
    m_driveStripManager->refresh();
}

void MainWindow::viewHidden( bool on )
{
    m_panes[ 0 ]->viewHidden( on );
    m_panes[ 1 ]->viewHidden( on );
}

void MainWindow::viewSameDirectory()
{
    m_targetPane->setFolder( m_sourcePane->folder()->clone() );
}

void MainWindow::openDirectory()
{
    m_sourcePane->openDirectory();
}

void MainWindow::openParent()
{
    m_sourcePane->openParent();
}

void MainWindow::openRoot()
{
    m_sourcePane->openRoot();
}

void MainWindow::openHome()
{
    int index = ( m_sourcePane == m_panes[ 0 ] ) ? 0 : 1;

    QString key = QString( "HomeDirectory%1" ).arg( index + 1 );
    restoreDirectory( index, key );

    m_panes[ index ]->activateView();
}

void MainWindow::gotoFile( const ShellPidl& folderPidl, const ShellItem& item )
{
    m_sourcePane->setFolder( new ShellFolder( folderPidl, m_sourcePane ) );
    m_sourcePane->activateView( item );
}

void MainWindow::swapPanes()
{
    ShellFolder* folder1 = m_panes[ 0 ]->folder()->clone();
    ShellFolder* folder2 = m_panes[ 1 ]->folder()->clone();
    m_panes[ 0 ]->setFolder( folder2 );
    m_panes[ 1 ]->setFolder( folder1 );
}

void MainWindow::copyToRightPane()
{
    m_panes[ 1 ]->setFolder( m_panes[ 0 ]->folder()->clone() );
}

void MainWindow::copyToLeftPane()
{
    m_panes[ 0 ]->setFolder( m_panes[ 1 ]->folder()->clone() );
}

void MainWindow::selectMask()
{
    LocalSettings* settings = application->applicationSettings();

    OperationDialog dialog( OperationDialog::WithPattern, this );

    dialog.setWindowTitle( tr( "Select Mask" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "select", 22 ) );
    dialog.setPrompt( tr( "Enter the pattern to select:" ) );

    dialog.setPatternList( settings->value( "Pattern" ).toStringList() );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    settings->setValue( "Pattern", dialog.patternList() );

    m_sourcePane->setPatternSelection( dialog.pattern(), true );
}

void MainWindow::selectAll()
{
    m_sourcePane->selectAll();
}

void MainWindow::unselectMask()
{
    LocalSettings* settings = application->applicationSettings();

    OperationDialog dialog( OperationDialog::WithPattern, this );

    dialog.setWindowTitle( tr( "Unselect Mask" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "unselect", 22 ) );
    dialog.setPrompt( tr( "Enter the pattern to unselect:" ) );

    dialog.setPatternList( settings->value( "Pattern" ).toStringList() );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    settings->setValue( "Pattern", dialog.patternList() );

    m_sourcePane->setPatternSelection( dialog.pattern(), false );
}

void MainWindow::unselectAll()
{
    m_sourcePane->unselectAll();
}

void MainWindow::invertSelection()
{
    m_sourcePane->invertSelection();
}

void MainWindow::renameCurrent()
{
    m_sourcePane->renameCurrent();
}

void MainWindow::viewCurrent()
{
    ShellItem item = m_sourcePane->currentItem();

    if ( !item.isValid() || !item.attributes().testFlag( ShellItem::Stream ) )
        return;

    LocalSettings* settings = application->applicationSettings();

    if ( settings->value( "InternalViewer" ).toBool() )
        m_viewManager->openView( m_sourcePane->folder()->itemPidl( item ) );
    else
        startTool( ViewerTool, m_sourcePane->folder(), m_sourcePane->currentItem() );
}

void MainWindow::viewSelected()
{
    QList<ShellItem> items;

    foreach ( ShellItem item, m_sourcePane->selectedItems() ) {
        if ( item.isValid() && item.attributes().testFlag( ShellItem::Stream ) )
            items.append( item );
    }

    if ( items.isEmpty() )
        return;

    LocalSettings* settings = application->applicationSettings();

    if ( settings->value( "InternalViewer" ).toBool() ) {
        QList<ShellPidl> pidls;
        foreach ( ShellItem item, items )
            pidls.append( m_sourcePane->folder()->itemPidl( item ) );
        m_viewManager->openView( pidls );
    } else {
        if ( items.count() == 1 )
            startTool( ViewerTool, m_sourcePane->folder(), items.first() );
        else
            QMessageBox::warning( this, tr( "Cannot view files" ), tr( "Only the internal viewer supports viewing multiple files." ) );
    }
}

void MainWindow::editCurrent()
{
    startTool( EditorTool, m_sourcePane->folder(), m_sourcePane->currentItem() ); 
}

void MainWindow::editNew()
{
    ShellFolder* folder = m_sourcePane->folder();
    ShellItem::Attributes attributes = folder->attributes();

    if ( !attributes.testFlag( ShellItem::FileSystem ) || !attributes.testFlag( ShellItem::Directory ) )
        return;

    OperationDialog dialog( OperationDialog::WithName | OperationDialog::CanEditName | OperationDialog::WithLocation, this );

    dialog.setWindowTitle( tr( "Create File" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "file-new", 22 ) );
    dialog.setPrompt( tr( "Create a new file:" ) );

    dialog.setLocation( folder->path() );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    QString name = dialog.name();

    if ( !folder->createFile( name, false ) )
        return;

    ShellItem item = folder->childItem( name );
    if ( !item.isValid() )
        return;

    m_sourcePane->setGotoItemName( item.name() );

    QString path = folder->itemPath( item );

    startTool( EditorTool, QString( "\"%1\"" ).arg( path ), folder->path() ); 
}

void MainWindow::startTool( Tool tool, ShellFolder* folder, const ShellItem& item )
{
    if ( !item.isValid() || !item.attributes().testFlag( ShellItem::FileSystem ) || item.attributes().testFlag( ShellItem::Directory ) )
        return;

    QString path = folder->itemPath( item );
    if ( path.isEmpty() )
        return;

    startTool( tool, QString( "\"%1\"" ).arg( path ), folder->path() );
}

void MainWindow::startTool( Tool tool, const QString& parameters, const QString& directory )
{
    QString path = toolPath( tool );

    if ( path.isEmpty() )
        return;

    HINSTANCE result = ShellExecute( (HWND)effectiveWinId(), NULL, (LPCWSTR)path.utf16(), (LPCWSTR)parameters.utf16(), (LPCWSTR)directory.utf16(), SW_SHOWNORMAL );

    if ( (int)result <= 32 ) {
        QString name = toolName( tool );
        QMessageBox::warning( this, tr( "Tool failed" ), tr( "The %1 tool could not be started.\nMake sure it is correctly configured in Saladin settings and try again." ).arg( name ) );
    }
}

QString MainWindow::toolPath( Tool tool )
{
    LocalSettings* settings = application->applicationSettings();

    QString path;
    switch ( tool ) {
        case ViewerTool:
            path = settings->value( "ViewerTool" ).toString();
            break;
        case EditorTool:
            path = settings->value( "EditorTool" ).toString();
            break;
        case ConsoleTool:
            path = settings->value( "ConsoleTool" ).toString();
            break;
        case DiffTool:
            path = settings->value( "DiffTool" ).toString();
            break;
    }

    if ( path.isEmpty() ) {
        QString name = toolName( tool );
        QMessageBox::warning( this, tr( "Missing tool" ), tr( "There is no %1 tool configured.\nSelect the tool in Saladin settings and try again." ).arg( name ) );
    }

    return path;
}

QString MainWindow::toolName( Tool tool )
{
    switch ( tool ) {
        case ViewerTool:
            return tr( "file viewer" );
        case EditorTool:
            return tr( "text editor" );
        case ConsoleTool:
            return tr( "console" );
        case DiffTool:
            return tr( "file compare" );
        default:
            return QString();
    }
}

void MainWindow::copySelected()
{
    transferItems( m_sourcePane->folder(), m_sourcePane->selectedItems(), m_targetPane->folder(), ShellSelection::Copy );
}

void MainWindow::cloneSelected()
{
    transferItems( m_sourcePane->folder(), m_sourcePane->selectedItems(), m_sourcePane->folder(), ShellSelection::Copy );
}

void MainWindow::moveSelected()
{
    transferItems( m_sourcePane->folder(), m_sourcePane->selectedItems(), m_targetPane->folder(), ShellSelection::Move );
}

void MainWindow::renameSelected()
{
    transferItems( m_sourcePane->folder(), m_sourcePane->selectedItems(), m_sourcePane->folder(), ShellSelection::Move );
}

void MainWindow::transferItems( ShellFolder* sourceFolder, const QList<ShellItem>& items, ShellFolder* targetFolder, ShellSelection::TransferType type )
{
    if ( items.isEmpty() )
        return;

    ShellSelection selection( sourceFolder, items, this );

    transferSelection( &selection, targetFolder, type, true );
}

void MainWindow::transferSelection( ShellSelection* selection, ShellFolder* targetFolder, ShellSelection::TransferType type, bool canRename )
{
    bool canTransfer = selection->canTransferTo( targetFolder, type );
    bool canDragDrop = !canTransfer && selection->canDragDropTo( targetFolder, type );

    if ( !canTransfer && !canDragDrop )
        return;

    bool sameTarget = targetFolder->isEqual( selection->folder() );

    if ( !canRename && sameTarget && type == ShellSelection::Move ) {
        QMessageBox::information( this, tr( "Drag & Drop" ), tr( "The source and target locations are the same." ) );
        return;
    }

    OperationDialog::Flags operationFlags;
    if ( sameTarget )
        operationFlags = OperationDialog::WithLocation;
    else
        operationFlags = OperationDialog::WithSource | OperationDialog::WithTarget;

    QList<ShellItem> items = selection->items();

    if ( canTransfer ) {
        operationFlags |= OperationDialog::WithCheckBox;
        if ( items.count() == 1 )
            operationFlags |= OperationDialog::WithName | OperationDialog::CanEditName;
        else if ( sameTarget )
            operationFlags |= OperationDialog::WithMultiRename;
        else
            operationFlags |= OperationDialog::CanRename;
    }

    OperationDialog dialog( operationFlags, this );

    if ( type == ShellSelection::Copy ) {
        if ( sameTarget ) {
            dialog.setWindowTitle( tr( "Clone" ) );
            dialog.setPromptPixmap( IconLoader::pixmap( "clone", 22 ) );
            if ( items.count() == 1 )
                dialog.setPrompt( tr( "Clone <b>%1</b>:" ).arg( items.first().name() ) );
            else
                dialog.setPrompt( tr( "Clone <b>%1</b> selected items:" ).arg( items.count() ) );
        } else {
            dialog.setWindowTitle( tr( "Copy" ) );
            dialog.setPromptPixmap( IconLoader::pixmap( "copy", 22 ) );
            if ( items.count() == 1 )
                dialog.setPrompt( tr( "Copy <b>%1</b>:" ).arg( items.first().name() ) );
            else
                dialog.setPrompt( tr( "Copy <b>%1</b> selected items:" ).arg( items.count() ) );
        }
    } else {
        if ( sameTarget ) {
            dialog.setWindowTitle( tr( "Rename" ) );
            dialog.setPromptPixmap( IconLoader::pixmap( "multi-rename", 22 ) );
            if ( items.count() == 1 )
                dialog.setPrompt( tr( "Rename <b>%1</b>:" ).arg( items.first().name() ) );
            else
                dialog.setPrompt( tr( "Rename <b>%1</b> selected items:" ).arg( items.count() ) );
        } else {
            dialog.setWindowTitle( tr( "Move" ) );
            dialog.setPromptPixmap( IconLoader::pixmap( "move", 22 ) );
            if ( items.count() == 1 )
                dialog.setPrompt( tr( "Move <b>%1</b>:" ).arg( items.first().name() ) );
            else
                dialog.setPrompt( tr( "Move <b>%1</b> selected items:" ).arg( items.count() ) );
        }
    }

    if ( items.count() == 1 ) {
        dialog.setName( items.first().name() );
    } else {
        QStringList names;
        foreach ( ShellItem item, items )
            names.append( item.name() );
        dialog.setNames( names );
    }

    if ( sameTarget ) {
        dialog.setLocation( selection->folder()->path() );
    } else {
        dialog.setSource( selection->folder()->path() );
        dialog.setTarget( targetFolder->path() );
    }

    if ( canTransfer )
        dialog.setCheckBoxText( tr( "O&verwrite without prompt" ) );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    bool done = false;

    if ( canTransfer ) {
        ShellSelection::Flags transferFlags = 0;
        if ( dialog.checkBoxChecked() )
            transferFlags |= ShellSelection::ForceOverwrite;

        QStringList newNames;
        if ( items.count() == 1 )
            newNames.append( dialog.name() );
        else
            newNames = dialog.names();

        done = selection->transferTo( targetFolder, type, transferFlags, newNames );
    } else {
        done = selection->dragDropTo( targetFolder, type );
    }

    if ( done )
        m_sourcePane->unselectAll();

    if ( done && sameTarget && items.count() == 1 )
        m_sourcePane->setGotoItemName( dialog.name() );
}

void MainWindow::createFolder()
{
    ShellFolder* folder = m_sourcePane->folder();

    if ( !folder->canCreateFolder() )
        return;

    OperationDialog dialog( OperationDialog::WithName | OperationDialog::CanEditName | OperationDialog::WithLocation, this );

    dialog.setWindowTitle( tr( "Create Folder" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "folder-new", 22 ) );
    dialog.setPrompt( tr( "Create a new folder:" ) );

    dialog.setLocation( folder->path() );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    QString name = dialog.name();

    bool done = false;

    ShellItem item = folder->childItem( name );
    if ( item.isValid() ) {
        name = item.name();
        done = item.attributes().testFlag( ShellItem::Directory );
    } else {
        done = folder->createFolder( name );
    }

    if ( done )
        m_sourcePane->setGotoItemName( name );
}

void MainWindow::moveToTrashCan()
{
    deleteItems( m_sourcePane->folder(), m_sourcePane->selectedItems(), 0 );
}

void MainWindow::deleteSelected()
{
    deleteItems( m_sourcePane->folder(), m_sourcePane->selectedItems(), ShellSelection::DeletePermanently );
}

void MainWindow::deleteItems( ShellFolder* folder, const QList<ShellItem>& items, ShellSelection::Flags flags )
{
    if ( items.isEmpty() )
        return;

    ShellSelection selection( folder, items, this );

    if ( selection.canDelete() )
        selection.deleteSelection( flags );
    else
        selection.invokeCommand( "delete" );
}

void MainWindow::openTerminal()
{
    ShellFolder* folder = m_sourcePane->folder();
    ShellItem::Attributes attributes = folder->attributes();

    if ( !attributes.testFlag( ShellItem::FileSystem ) || !attributes.testFlag( ShellItem::Directory ) )
        return;

    startTool( ConsoleTool, QString(), folder->path() );
}

void MainWindow::packToZip()
{
    QList<ShellItem> items = m_sourcePane->selectedItems();
    if ( items.isEmpty() )
        return;

    ShellFolder* targetFolder = m_targetPane->folder();
    ShellItem::Attributes attributes = targetFolder ->attributes();

    if ( !attributes.testFlag( ShellItem::FileSystem ) || !attributes.testFlag( ShellItem::Directory ) )
        return;

    OperationDialog dialog( OperationDialog::WithName | OperationDialog::CanEditName | OperationDialog::WithTarget | OperationDialog::WithCheckBox, this );

    dialog.setWindowTitle( tr( "Pack To Zip" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "pack", 22 ) );

    if ( items.count() == 1 ) {
        QString itemName = items.first().name();
        dialog.setPrompt( tr( "Pack <b>%1</b>:" ).arg( itemName ) );

        int pos = itemName.lastIndexOf( '.' );
        if ( pos > 0 )
            itemName.truncate( pos );
        dialog.setName( itemName + ".zip" );
    } else {
        dialog.setPrompt( tr( "Pack <b>%1</b> selected items:" ).arg( items.count() ) );
        dialog.setName( m_sourcePane->folder()->name() + ".zip" );
    }

    dialog.setTarget( targetFolder->path() );

    dialog.setCheckBoxText( tr( "O&verwrite without prompt" ) );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    QString name = dialog.name();

    static const char zipData[ 22 ] = { 'P', 'K', 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    if ( !targetFolder->createFile( name, dialog.checkBoxChecked(), zipData, sizeof( zipData ) ) )
        return;

    ShellItem item = targetFolder->childItem( name );
    if ( !item.isValid() )
        return;

    ShellFolder* zipFolder = targetFolder->openFolder( item );
    if ( !zipFolder )
        return;

    ShellSelection selection( m_sourcePane->folder(), items, this );
    bool done = selection.dragDropTo( zipFolder, ShellSelection::Copy );

    delete zipFolder;

    if ( done )
        m_sourcePane->unselectAll();
}

void MainWindow::browse()
{
    m_sourcePane->browse();
}

void MainWindow::openFtpSite()
{
    OpenFtpDialog dialog( this );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    m_sourcePane->setDirectory( dialog.path() );
}

void MainWindow::calculateSize()
{
    m_sourcePane->calculateSize();
}

void MainWindow::compareFiles()
{
    QStringList paths;

    QList<ShellItem> items = m_sourcePane->selectedItems();
    if ( items.count() == 1 ) {
        ShellItem item = items.first();
        if ( !item.isValid() || !item.attributes().testFlag( ShellItem::FileSystem ) || item.attributes().testFlag( ShellItem::Directory ) )
            return;
        QString path = m_sourcePane->folder()->itemPath( item );
        if ( path.isEmpty() )
            return;
        paths.append( path );

        items = m_targetPane->selectedItems();
        if ( items.count() == 1 ) {
            ShellItem item = items.first();
            if ( !item.isValid() || !item.attributes().testFlag( ShellItem::FileSystem ) || item.attributes().testFlag( ShellItem::Directory ) )
                return;
            QString path = m_targetPane->folder()->itemPath( item );
            if ( path.isEmpty() )
                return;
            paths.prepend( path );
        }
    } else if ( items.count() == 2 ) {
        for ( int i = 0; i < 2; i++ ) {
            ShellItem item = items.at( i );
            if ( !item.isValid() || !item.attributes().testFlag( ShellItem::FileSystem ) || item.attributes().testFlag( ShellItem::Directory ) )
                return;
            QString path = m_sourcePane->folder()->itemPath( item );
            if ( path.isEmpty() )
                return;
            paths.append( path );
        }
    }

    if ( paths.count() != 2 )
        return;

    QString parameters = QString( "\"%1\" \"%2\"" ).arg( paths.at( 0 ), paths.at( 1 ) );

    startTool( DiffTool, parameters, m_sourcePane->folder()->path() );
}

void MainWindow::compareDirectories()
{
    if ( !m_sourcePane->folder()->isEqual( m_targetPane->folder() ) )
        m_sourcePane->compareWith( m_targetPane->items() );
}

void MainWindow::search()
{
    SearchDialog* dialog = new SearchDialog( m_sourcePane->folder(), this );
    dialog->setAttribute( Qt::WA_DeleteOnClose, true );
    dialog->setWindowModality( Qt::WindowModal );
    dialog->show();
}

void MainWindow::explore()
{
    m_sourcePane->folder()->explore();
}

void MainWindow::showHistory()
{
    m_sourcePane->showHistory();
}

void MainWindow::showBookmarks()
{
    m_sourcePane->showBookmarks();
}

void MainWindow::addBookmark()
{
    ShellFolder* folder = m_sourcePane->folder();

    OperationDialog::Flags flags = OperationDialog::WithName | OperationDialog::CanEditName | OperationDialog::WithLocation;
    if ( !folder->password().isEmpty() )
        flags |= OperationDialog::WithCheckBox;

    OperationDialog dialog( flags, this );

    dialog.setWindowTitle( tr( "Add Bookmark" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "bookmark", 22 ) );
    dialog.setPrompt( tr( "Add current directory to the list of bookmarks:" ) );

    dialog.setName( folder->path() );
    dialog.setLocation( folder->path() );

    dialog.setCheckBoxText( tr( "&Remember password" ) );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    Bookmark bookmark( dialog.name(), folder, dialog.checkBoxChecked() );
    application->addBookmark( bookmark );
}

void MainWindow::editBookmarks()
{
    BookmarksDialog dialog( this );
    dialog.exec();
}

void MainWindow::showDrivesMenu1()
{
    m_panes[ 0 ]->showDrivesMenu();
}

void MainWindow::showDrivesMenu2()
{
    m_panes[ 1 ]->showDrivesMenu();
}

void MainWindow::showProperties()
{
    invokeCommand( m_sourcePane->folder(), m_sourcePane->selectedItems(), "properties" );
}

void MainWindow::otherOpenFolder()
{
    ShellFolder* folder = m_sourcePane->folder()->openFolder( m_sourcePane->currentItem() );
    if ( folder )
        m_targetPane->setFolder( folder );
}

void MainWindow::otherOpenParent()
{
    ShellItem item;
    ShellFolder* folder = m_sourcePane->folder()->parentFolder( item );
    if ( folder )
        m_targetPane->setFolder( folder );
}

void MainWindow::hideToolStrip()
{
    builder()->unregisterToolStrip( "stripMain" );

    createMenuBar();

    application->applicationSettings()->setValue( "HideToolStrip", true );
}

void MainWindow::showToolStrip()
{
    createToolStrip();

    application->applicationSettings()->setValue( "HideToolStrip", false );
}

void MainWindow::createToolStrip()
{
    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    strip->addAuxiliaryAction( this->action( "hideToolStrip" ) );
    strip->addAuxiliaryAction( this->action( "configure" ) );
    strip->addAuxiliaryAction( this->action( "about" ) );
    setMenuWidget( strip );

    builder()->registerToolStrip( "stripMain", strip );
}

void MainWindow::createMenuBar()
{
    QMenuBar* menuBar = new QMenuBar( this );
    setMenuWidget( menuBar );

    menuBar->addMenu( builder()->contextMenu( "menuBarEdit" ) )->setText( tr( "&Edit" ) );
    menuBar->addMenu( builder()->contextMenu( "menuBarView" ) )->setText( tr( "&View" ) );
    menuBar->addMenu( builder()->contextMenu( "menuBarSelect" ) )->setText( tr( "&Selection" ) );
    menuBar->addMenu( builder()->contextMenu( "menuBarFunctions" ) )->setText( tr( "&Functions" ) );
    menuBar->addMenu( builder()->contextMenu( "menuBarTools" ) )->setText( tr( "&Tools" ) );
    menuBar->addMenu( builder()->contextMenu( "menuBarHelp" ) )->setText( tr( "&Help" ) );

    QToolButton* button = new QToolButton( menuBar );
    button->setAutoRaise( true );
    button->setFocusPolicy( Qt::NoFocus );
    button->setToolButtonStyle( Qt::ToolButtonIconOnly );
    button->setIconSize( QSize( 16, 16 ) );
    button->setDefaultAction( action( "showToolStrip" ) );

    menuBar->setCornerWidget( button, Qt::TopRightCorner );
}

void MainWindow::loadIcons()
{
    action( "about" )->setIcon( IconLoader::icon( "about" ) );
    action( "releaseNotes" )->setIcon( IconLoader::icon( "web" ) );
    action( "showQuickGuide" )->setIcon( IconLoader::icon( "help" ) );
    action( "configure" )->setIcon( IconLoader::icon( "configure" ) );
    action( "paste" )->setIcon( IconLoader::icon( "edit-paste" ) );
    action( "cut" )->setIcon( IconLoader::icon( "edit-cut" ) );
    action( "popupEditCopy" )->setIcon( IconLoader::icon( "edit-copy" ) );
    action( "copy" )->setIcon( IconLoader::icon( "edit-copy" ) );
    action( "copyNames" )->setIcon( IconLoader::icon( "copy-names" ) );
    action( "refresh" )->setIcon( IconLoader::icon( "refresh" ) );
    action( "refreshDrives" )->setIcon( IconLoader::icon( "refresh" ) );
    action( "viewHidden" )->setIcon( IconLoader::icon( "view-hidden" ) );
    action( "viewSameDirectory" )->setIcon( IconLoader::icon( "pane-same" ) );
    action( "swapPanes" )->setIcon( IconLoader::icon( "pane-swap" ) );
    action( "openHome" )->setIcon( IconLoader::icon( "home" ) );
    action( "openRoot" )->setIcon( IconLoader::icon( "arrow-top" ) );
    action( "openParent" )->setIcon( IconLoader::icon( "arrow-up" ) );
    action( "copyToRightPane" )->setIcon( IconLoader::icon( "arrow-right" ) );
    action( "copyToLeftPane" )->setIcon( IconLoader::icon( "arrow-left" ) );
    action( "selectMask" )->setIcon( IconLoader::icon( "select" ) );
    action( "selectAll" )->setIcon( IconLoader::icon( "select-all" ) );
    action( "unselectMask" )->setIcon( IconLoader::icon( "unselect" ) );
    action( "unselectAll" )->setIcon( IconLoader::icon( "unselect-all" ) );
    action( "invertSelection" )->setIcon( IconLoader::icon( "select-invert" ) );
    action( "renameCurrent" )->setIcon( IconLoader::icon( "rename" ) );
    action( "popupView" )->setIcon( IconLoader::icon( "view" ) );
    action( "viewCurrent" )->setIcon( IconLoader::icon( "view" ) );
    action( "viewSelected" )->setIcon( IconLoader::icon( "view-selected" ) );
    action( "popupEdit" )->setIcon( IconLoader::icon( "edit" ) );
    action( "editCurrent" )->setIcon( IconLoader::icon( "edit" ) );
    action( "editNew" )->setIcon( IconLoader::icon( "file-new" ) );
    action( "popupCopy" )->setIcon( IconLoader::icon( "copy" ) );
    action( "copySelected" )->setIcon( IconLoader::icon( "copy" ) );
    action( "cloneSelected" )->setIcon( IconLoader::icon( "clone" ) );
    action( "popupMove" )->setIcon( IconLoader::icon( "move" ) );
    action( "moveSelected" )->setIcon( IconLoader::icon( "move" ) );
    action( "renameSelected" )->setIcon( IconLoader::icon( "multi-rename" ) );
    action( "createFolder" )->setIcon( IconLoader::icon( "folder-new" ) );
    action( "popupDelete" )->setIcon( IconLoader::icon( "trashcan" ) );
    action( "moveToTrashCan" )->setIcon( IconLoader::icon( "trashcan" ) );
    action( "deleteSelected" )->setIcon( IconLoader::icon( "delete" ) );
    action( "openTerminal" )->setIcon( IconLoader::icon( "terminal" ) );
    action( "packToZip" )->setIcon( IconLoader::icon( "pack" ) );
    action( "popupOpen" )->setIcon( IconLoader::icon( "browse" ) );
    action( "openDirectory" )->setIcon( IconLoader::icon( "open" ) );
    action( "browse" )->setIcon( IconLoader::icon( "browse" ) );
    action( "openFtpSite" )->setIcon( IconLoader::icon( "ftp" ) );
    action( "calculateSize" )->setIcon( IconLoader::icon( "calculate" ) );
    action( "popupCompare" )->setIcon( IconLoader::icon( "compare" ) );
    action( "compareFiles" )->setIcon( IconLoader::icon( "compare" ) );
    action( "compareDirectories" )->setIcon( IconLoader::icon( "folder-compare" ) );
    action( "search" )->setIcon( IconLoader::icon( "find" ) );
    action( "explore" )->setIcon( IconLoader::icon( "explore" ) );
    action( "showHistory" )->setIcon( IconLoader::icon( "history" ) );
    action( "showBookmarks" )->setIcon( IconLoader::icon( "bookmark" ) );
    action( "addBookmark" )->setIcon( IconLoader::icon( "bookmark" ) );
    action( "editBookmarks" )->setIcon( IconLoader::icon( "edit" ) );
    action( "hideToolStrip" )->setIcon( IconLoader::icon( "arrow-hide" ) );
    action( "showToolStrip" )->setIcon( IconLoader::icon( "arrow-down" ) );
}
