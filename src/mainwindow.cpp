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

#include "mainwindow.h"
#include "application.h"
#include "panewidget.h"
#include "drivestripmanager.h"
#include "operationdialog.h"
#include "openftpdialog.h"

#include "shell/shellfolder.h"
#include "shell/shellselection.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "xmlui/toolstrip.h"
#include "xmlui/builder.h"

MainWindow::MainWindow() : QMainWindow(),
    m_sourcePane( NULL ),
    m_targetPane( NULL )
{
    m_panes[ 0 ] = m_panes[ 1 ] = NULL;

    m_driveStripManager = new DriveStripManager( this );
}

MainWindow::~MainWindow()
{
    application->removeEventFilter( this );

    LocalSettings* settings = application->applicationSettings();
    settings->setValue( "MainWindowGeometry", saveGeometry() );
}

void MainWindow::initialize()
{
    setWindowTitle( tr( "Saladin" ) );

    QAction* action;

    action = new QAction( IconLoader::icon( "help" ), tr( "About Saladin" ), this );
    connect( action, SIGNAL( triggered() ), application, SLOT( about() ) );
    setAction( "about", action );

    action = new QAction( IconLoader::icon( "edit-paste" ), tr( "Paste" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_V ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( paste() ) );
    setAction( "paste", action );

    action = new QAction( IconLoader::icon( "edit-cut" ), tr( "Cut" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_X ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( cut() ) );
    setAction( "cut", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "Copy" ), this );
    setAction( "popupEditCopy", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "Copy Files" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_C ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( copy() ) );
    setAction( "copy", action );

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "Copy File Names" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyNames() ) );
    setAction( "copyNames", action );

    action = new QAction( IconLoader::icon( "refresh" ), tr( "Refresh" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( refresh() ) );
    setAction( "refresh", action );

    action = new QAction( IconLoader::icon( "view-hidden" ), tr( "Hidden Files" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_H ) );
    action->setCheckable( true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( viewHidden( bool ) ) );
    setAction( "viewHidden", action );

    action = new QAction( IconLoader::icon( "pane-same" ), tr( "Same Directory" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_E ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( viewSameDirectory() ) );
    setAction( "viewSameDirectory", action );

    action = new QAction( IconLoader::icon( "pane-swap" ), tr( "Swap Panels" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_U ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( swapPanes() ) );
    setAction( "swapPanes", action );

    action = new QAction( IconLoader::icon( "arrow-top" ), tr( "Root Directory" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Backslash ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openRoot() ) );
    setAction( "openRoot", action );

    action = new QAction( IconLoader::icon( "arrow-up" ), tr( "Parent Directory" ), this );
    connect( action, SIGNAL( triggered() ), this, SLOT( openParent() ) );
    setAction( "openParent", action );

    action = new QAction( IconLoader::icon( "arrow-right" ), "Left -> Right", this );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyToRightPane() ) );
    setAction( "copyToRightPane", action );

    action = new QAction( IconLoader::icon( "arrow-left" ), "Left <- Right", this );
    connect( action, SIGNAL( triggered() ), this, SLOT( copyToLeftPane() ) );
    setAction( "copyToLeftPane", action );

    action = new QAction( IconLoader::icon( "select" ), tr( "Select Mask" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Equal ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( selectMask() ) );
    setAction( "selectMask", action );

    action = new QAction( IconLoader::icon( "select-all" ), tr( "Select All" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_A ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
    setAction( "selectAll", action );

    action = new QAction( IconLoader::icon( "unselect" ), tr( "Unselect Mask" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Minus ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( unselectMask() ) );
    setAction( "unselectMask", action );

    action = new QAction( IconLoader::icon( "unselect-all" ), tr( "Unselect All" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_A ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( unselectAll() ) );
    setAction( "unselectAll", action );

    action = new QAction( IconLoader::icon( "select-invert" ), tr( "Invert Selection" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_I ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( invertSelection() ) );
    setAction( "invertSelection", action );

    action = new QAction( IconLoader::icon( "rename" ), tr( "Rename" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F2 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( renameCurrent() ) );
    setAction( "renameCurrent", action );

    action = new QAction( IconLoader::icon( "view" ), tr( "View" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F3 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( viewCurrent() ) );
    setAction( "viewCurrent", action );

    action = new QAction( IconLoader::icon( "edit" ), tr( "Edit / Create" ), this );
    setAction( "popupEdit", action );

    action = new QAction( IconLoader::icon( "edit" ), tr( "Edit File" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F4 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editCurrent() ) );
    setAction( "editCurrent", action );

    action = new QAction( IconLoader::icon( "file-new" ), tr( "Create && Edit" ), this );
    action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_F4 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( editNew() ) );
    setAction( "editNew", action );

    action = new QAction( IconLoader::icon( "copy" ), tr( "Copy / Clone" ), this );
    setAction( "popupCopy", action );

    action = new QAction( IconLoader::icon( "copy" ), tr( "Copy To Target Directory" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F5 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( copySelected() ) );
    setAction( "copySelected", action );

    action = new QAction( IconLoader::icon( "clone" ), tr( "Clone In Source Directory" ), this );
    action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_F5 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( cloneSelected() ) );
    setAction( "cloneSelected", action );

    action = new QAction( IconLoader::icon( "move" ), tr( "Move / Rename" ), this );
    setAction( "popupMove", action );

    action = new QAction( IconLoader::icon( "move" ), tr( "Move To Target Directory" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F6 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( moveSelected() ) );
    setAction( "moveSelected", action );

    action = new QAction( IconLoader::icon( "multi-rename" ), tr( "Rename In Source Directory" ), this );
    action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_F6 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( renameSelected() ) );
    setAction( "renameSelected", action );

    action = new QAction( IconLoader::icon( "folder-new" ), tr( "Create Folder" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F7 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( createFolder() ) );
    setAction( "createFolder", action );

    action = new QAction( IconLoader::icon( "trashcan" ), tr( "Delete" ), this );
    setAction( "popupDelete", action );

    action = new QAction( IconLoader::icon( "trashcan" ), tr( "Move To Recycle Bin" ), this );
    action->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::Key_F8 ) << QKeySequence( Qt::Key_Delete ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( moveToTrashCan() ) );
    setAction( "moveToTrashCan", action );

    action = new QAction( IconLoader::icon( "delete" ), tr( "Delete Permanently" ), this );
    action->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::SHIFT + Qt::Key_F8 ) << QKeySequence( Qt::SHIFT + Qt::Key_Delete ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( deleteSelected() ) );
    setAction( "deleteSelected", action );

    action = new QAction( IconLoader::icon( "terminal" ), tr( "Open Console" ), this );
    action->setShortcut( QKeySequence( Qt::Key_F9 ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openTerminal() ) );
    setAction( "openTerminal", action );

    action = new QAction( IconLoader::icon( "pack" ), tr( "Pack" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_P ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( packToZip() ) );
    setAction( "packToZip", action );

    action = new QAction( IconLoader::icon( "browse" ), tr( "Open" ), this );
    setAction( "popupOpen", action );

    action = new QAction( IconLoader::icon( "open" ), tr( "Open Directory" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_O ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openDirectory() ) );
    setAction( "openDirectory", action );

    action = new QAction( IconLoader::icon( "browse" ), tr( "Browse For Folder" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_O ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( browse() ) );
    setAction( "browse", action );

    action = new QAction( IconLoader::icon( "ftp" ), tr( "Open FTP Site" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( openFtpSite() ) );
    setAction( "openFtpSite", action );

    action = new QAction( IconLoader::icon( "calculate" ), tr( "Show Size" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_L ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( calculateSize() ) );
    setAction( "calculateSize", action );

    action = new QAction( IconLoader::icon( "compare" ), tr( "Compare" ), this );
    setAction( "popupCompare", action );

    action = new QAction( IconLoader::icon( "compare" ), tr( "Compare Files" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( compareFiles() ) );
    setAction( "compareFiles", action );

    action = new QAction( IconLoader::icon( "folder-compare" ), tr( "Compare Directories" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_M ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( compareDirectories() ) );
    setAction( "compareDirectories", action );

    action = new QAction( IconLoader::icon( "find" ), tr( "Search" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( search() ) );
    setAction( "search", action );

    action = new QAction( IconLoader::icon( "explore" ), tr( "Explore" ), this );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_E ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( explore() ) );
    setAction( "explore", action );

    setTitle( "sectionFunctions", tr( "Functions" ) );
    setTitle( "sectionClipboard", tr( "Clipboard" ) );
    setTitle( "sectionView", tr( "View" ) );
    setTitle( "sectionSelect", tr( "Select" ) );
    setTitle( "sectionTools", tr( "Tools" ) );

    setPopupMenu( "popupEdit", "menuEdit", "editCurrent" );
    setPopupMenu( "popupCopy", "menuCopy", "copySelected" );
    setPopupMenu( "popupMove", "menuMove", "moveSelected" );
    setPopupMenu( "popupDelete", "menuDelete", "moveToTrashCan" );
    setPopupMenu( "popupEditCopy", "menuEditCopy", "copy" );
    setPopupMenu( "popupOpen", "menuOpen", "browse" );
    setPopupMenu( "popupCompare", "menuCompare", "compareFiles" );

    loadXmlUiFile( ":/resources/mainwindow.xml" );

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip( this );
    strip->addAuxiliaryAction( this->action( "about" ) );
    setMenuWidget( strip );

    XmlUi::Builder* builder = new XmlUi::Builder( this );
    builder->addClient( this );

    builder->registerToolStrip( "stripMain", strip );

    QShortcut* shortcut;

    shortcut = new QShortcut( Qt::ALT + Qt::Key_F1, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( showDrivesMenu1() ) );

    shortcut = new QShortcut( Qt::ALT + Qt::Key_F2, this );
    connect( shortcut, SIGNAL( activated() ), this, SLOT( showDrivesMenu2() ) );

    QWidget* widget = new QWidget( this );
    setCentralWidget( widget );

    m_panes[ 0 ] = new PaneWidget( PaneWidget::LeftPane, widget );
    m_panes[ 1 ] = new PaneWidget( PaneWidget::RightPane, widget );

    QHBoxLayout* layout = new QHBoxLayout( widget );
    layout->setContentsMargins( 3, 0, 3, 0 );
    layout->setSpacing( 5 );

    layout->addWidget( m_panes[ 0 ] );
    layout->addWidget( m_panes[ 1 ] );

    LocalSettings* settings = application->applicationSettings();
    if ( settings->contains( "MainWindowGeometry" ) )
        restoreGeometry( settings->value( "MainWindowGeometry" ).toByteArray() );
    else
        resize( QSize( 1020, 680 ) );

    connect( m_panes[ 0 ], SIGNAL( headerSectionResized( int, int ) ), m_panes[ 1 ], SLOT( resizeHeaderSection( int, int ) ) );
    connect( m_panes[ 1 ], SIGNAL( headerSectionResized( int, int ) ), m_panes[ 0 ], SLOT( resizeHeaderSection( int, int ) ) );
    connect( m_panes[ 0 ], SIGNAL( headerSectionMoved( int, int ) ), m_panes[ 1 ], SLOT( moveHeaderSection( int, int ) ) );
    connect( m_panes[ 1 ], SIGNAL( headerSectionMoved( int, int ) ), m_panes[ 0 ], SLOT( moveHeaderSection( int, int ) ) );

    application->installEventFilter( this );

    m_panes[ 0 ]->setFocus();
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

void MainWindow::setSourcePane( int index )
{
    if ( m_sourcePane == m_panes[ index ] )
        return;

    m_sourcePane = m_panes[ index ];
    m_targetPane = m_panes[ 1 - index ];

    m_sourcePane->setSourcePane( true );
    m_targetPane->setSourcePane( false );
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
    OperationDialog dialog( OperationDialog::WithPattern, this );

    dialog.setWindowTitle( tr( "Select Mask" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "select", 22 ) );
    dialog.setPrompt( tr( "Enter the pattern to select:" ) );

    if ( dialog.exec() != QDialog::Accepted )
        return;

    m_sourcePane->setPatternSelection( dialog.pattern(), true );
}

void MainWindow::selectAll()
{
    m_sourcePane->selectAll();
}

void MainWindow::unselectMask()
{
    OperationDialog dialog( OperationDialog::WithPattern, this );

    dialog.setWindowTitle( tr( "Unselect Mask" ) );
    dialog.setPromptPixmap( IconLoader::pixmap( "unselect", 22 ) );
    dialog.setPrompt( tr( "Enter the pattern to unselect:" ) );

    if ( dialog.exec() != QDialog::Accepted )
        return;

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
    startTool( Viewer, m_sourcePane->folder(), m_sourcePane->currentItem() ); 
}

void MainWindow::editCurrent()
{
    startTool( Editor, m_sourcePane->folder(), m_sourcePane->currentItem() ); 
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

    if ( !folder->createFile( name ) )
        return;

    ShellItem item = folder->childItem( name );
    if ( !item.isValid() )
        return;

    startTool( Editor, folder->itemPath( item ), folder->path() ); 
}

void MainWindow::startTool( Tool tool, ShellFolder* folder, ShellItem item )
{
    if ( !item.isValid() || !item.attributes().testFlag( ShellItem::FileSystem ) || item.attributes().testFlag( ShellItem::Directory ) )
        return;

    QString path = folder->itemPath( item );
    if ( path.isEmpty() )
        return;

    startTool( tool, path, folder->path() );
}

void MainWindow::startTool( Tool tool, const QString& path, const QString& directory )
{
    QString parameters = QString( "\"%1\"" ).arg( path );

    wchar_t buffer[ MAX_PATH ];
    if ( !SHGetSpecialFolderPath( effectiveWinId(), buffer, CSIDL_PROGRAM_FILESX86, FALSE ) )
        return;

    QString programFiles = QString::fromWCharArray( buffer );

    QString toolPath;
    switch ( tool ) {
        case Viewer:
            toolPath = programFiles + "\\Universal Viewer\\Viewer.exe";
            break;
        case Editor:
            toolPath = programFiles + "\\Notepad++\\notepad++.exe";
            break;
    }

    ShellExecute( effectiveWinId(), NULL, toolPath.utf16(), parameters.utf16(), directory.utf16(), SW_SHOWNORMAL );
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

    bool canTransfer = selection.canTransferTo( targetFolder, type );
    bool canDragDrop = !canTransfer && selection.canDragDropTo( targetFolder, type );

    if ( !canTransfer && !canDragDrop )
        return;

    bool sameTarget = targetFolder->match( sourceFolder );

    OperationDialog::Flags operationFlags;
    if ( sameTarget )
        operationFlags = OperationDialog::WithLocation;
    else
        operationFlags = OperationDialog::WithSource | OperationDialog::WithTarget;

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
        dialog.setLocation( sourceFolder->path() );
    } else {
        dialog.setSource( sourceFolder->path() );
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

        done = selection.transferTo( targetFolder, type, transferFlags, newNames );
    } else {
        done = selection.dragDropTo( targetFolder, type );
    }

    if ( done )
        m_sourcePane->unselectAll();
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

    ShellItem item = folder->childItem( name );
    if ( !item.isValid() )
        folder->createFolder( name );
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

    QString path = folder->path();

    ShellExecute( effectiveWinId(), NULL, L"cmd.exe", NULL, path.utf16(), SW_SHOWNORMAL );
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

    OperationDialog dialog( OperationDialog::WithName | OperationDialog::CanEditName | OperationDialog::WithTarget, this );

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

    if ( dialog.exec() != QDialog::Accepted )
        return;

    QString name = dialog.name();

    static const char zipData[ 22 ] = { 'P', 'K', 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    if ( !targetFolder->createFile( name, zipData, sizeof( zipData ) ) )
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
    // TODO
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
            paths.append( path );
        }
    } else if ( items.count() == 2 ) {
        for ( int i = 0; i < 2; i++ ) {
            ShellItem item = items.at( i);
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

    wchar_t buffer[ MAX_PATH ];
    if ( !SHGetSpecialFolderPath( effectiveWinId(), buffer, CSIDL_PROGRAM_FILES, FALSE ) )
        return;

    QString toolPath = QString::fromWCharArray( buffer ) + "\\TortoiseSVN\\bin\\TortoiseMerge.exe";

    QString directory = m_sourcePane->folder()->path();

    ShellExecute( effectiveWinId(), NULL, toolPath.utf16(), parameters.utf16(), directory.utf16(), SW_SHOWNORMAL );
}

void MainWindow::compareDirectories()
{
    // TODO
}

void MainWindow::search()
{
    m_sourcePane->folder()->search();
}

void MainWindow::explore()
{
    m_sourcePane->folder()->explore();
}

void MainWindow::showDrivesMenu1()
{
    m_panes[ 0 ]->showDrivesMenu();
}

void MainWindow::showDrivesMenu2()
{
    m_panes[ 1 ]->showDrivesMenu();
}
