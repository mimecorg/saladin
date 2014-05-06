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

#include "panewidget.h"
#include "application.h"
#include "mainwindow.h"
#include "folderitemview.h"
#include "folderitemmodel.h"
#include "folderitemdelegate.h"
#include "drivestripmanager.h"
#include "openftpdialog.h"

#include "shell/shellfolder.h"
#include "shell/shellitem.h"
#include "shell/shellpidl.h"
#include "shell/shelldropdata.h"
#include "utils/elidedlabel.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"

PaneWidget::PaneWidget( PaneLocation location, QWidget* parent ) : QWidget( parent ),
    m_location( location ),
    m_strip( NULL ),
    m_view( NULL ),
    m_model( NULL ),
    m_isSource( false ),
    m_movingSection( false ),
    m_historyIndex( 0 ),
    m_lockHistory( false ),
    m_dropData( NULL )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setSpacing( 0 );
    layout->setMargin( 0 );

    m_strip = new XmlUi::ToolStrip( this );
    layout->addWidget( m_strip );
    layout->addSpacing( 3 );

    m_strip->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_strip, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( stripContextMenuRequested( const QPoint& ) ) );

    m_strip->addAuxiliaryAction( mainWindow->action( "openRoot" ) );
    m_strip->addAuxiliaryAction( mainWindow->action( "openParent" ) );
    if ( location == LeftPane )
        m_strip->addAuxiliaryAction( mainWindow->action( "copyToRightPane" ) );
    else
        m_strip->addAuxiliaryAction( mainWindow->action( "copyToLeftPane" ) );

    QHBoxLayout* editLayout = new QHBoxLayout();

    QFrame* editFrame = new QFrame( this );
    editFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    editLayout->addWidget( editFrame );
    editLayout->addSpacing( 3 );

    QHBoxLayout* innerLayout = new QHBoxLayout( editFrame );
    innerLayout->setMargin( 0 );

    m_edit = new QLineEdit( editFrame );
    m_edit->setFocusPolicy( Qt::ClickFocus );
    m_edit->setFrame( false );
    m_edit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    innerLayout->addWidget( m_edit );

    updateEditPalette();

    connect( m_edit, SIGNAL( returnPressed() ), this, SLOT( changeDirectory() ) );

    m_bookmarkButton = new XmlUi::ActionButton( parent );
    m_bookmarkButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
    m_bookmarkButton->setIconSize( QSize( 16, 16 ) );
    m_bookmarkButton->setDefaultAction( mainWindow->action( "showBookmarks" ) );
    m_bookmarkButton->adjustText();
    editLayout->addWidget( m_bookmarkButton );

    m_historyButton = new XmlUi::ActionButton( parent );
    m_historyButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
    m_historyButton->setIconSize( QSize( 16, 16 ) );
    m_historyButton->setDefaultAction( mainWindow->action( "showHistory" ) );
    m_historyButton->adjustText();
    editLayout->addWidget( m_historyButton );

    addAction( mainWindow->action( "addBookmark" ) );

    layout->addLayout( editLayout );
    layout->addSpacing( 3 );

    m_view = new FolderItemView( this );
    m_view->setSelectionMode( QAbstractItemView::NoSelection );
    m_view->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_view->setRootIsDecorated( false );
    m_view->setAlternatingRowColors( true );
    m_view->setAllColumnsShowFocus( true );
    m_view->setUniformRowHeights( true );
    m_view->header()->setStretchLastSection( false );
    m_view->setContextMenuPolicy( Qt::CustomContextMenu );
    m_view->viewport()->setAcceptDrops( true );
    m_view->setAutoScroll( true );
    m_view->setAutoScrollMargin( 50 );
    layout->addWidget( m_view, 1 );

    connect( m_view, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( viewContextMenuRequested( const QPoint& ) ) );

    connect( m_view->header(), SIGNAL( sectionResized( int, int, int ) ), this, SLOT( sectionResized( int, int, int ) ) );
    connect( m_view->header(), SIGNAL( sectionMoved( int, int, int ) ), this, SLOT( sectionMoved( int, int, int ) ) );

    m_model = new FolderItemModel( this );
    m_view->setModel( m_model );

    FolderItemDelegate* itemDelegate = new FolderItemDelegate( this );
    m_view->setItemDelegate( itemDelegate );

    m_view->setSortingEnabled( true );
    m_view->sortByColumn( 0, Qt::AscendingOrder );

    m_renameTimer = new QTimer( this );
    m_renameTimer->setInterval( qApp->doubleClickInterval() + 200 );
    m_renameTimer->setSingleShot( true );

    connect( m_renameTimer, SIGNAL( timeout() ), this, SLOT( renameTimeout() ) );

    QStatusBar* status = new QStatusBar( this );
    status->setSizeGripEnabled( false );
    layout->addWidget( status );

    m_selectionStatus = new ElidedLabel( status );
    status->addWidget( m_selectionStatus, 1 );

    m_driveStatus = new ElidedLabel( status );
    status->addWidget( m_driveStatus, 1 );

    connect( m_model, SIGNAL( modelReset() ), this, SLOT( updateStatus() ) );
    connect( m_model, SIGNAL( layoutChanged() ), this, SLOT( updateStatus() ) );
    connect( m_model, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( updateStatus() ) );

    m_edit->installEventFilter( this );
    m_view->installEventFilter( this );
    m_view->viewport()->installEventFilter( this );

    setFocusProxy( m_view );
}

PaneWidget::~PaneWidget()
{
}

void PaneWidget::restoreSettings()
{
    LocalSettings* settings = application->applicationSettings();

    QString key = QString( "HeaderState%1" ).arg( m_location + 1 );
    if ( settings->contains( key ) ) {
        m_view->header()->restoreState( settings->value( key ).toByteArray() );
    } else {
        m_view->setColumnWidth( 0, 210 );
        m_view->setColumnWidth( 1, 90 );
        m_view->setColumnWidth( 2, 110 );
        m_view->setColumnWidth( 3, 70 );
    }
}

void PaneWidget::saveSettings()
{
    LocalSettings* settings = application->applicationSettings();

    QString key = QString( "HeaderState%1" ).arg( m_location + 1 );
    settings->setValue( key, m_view->header()->saveState() );
}

void PaneWidget::populateDrives()
{
    mainWindow->driveStripManager()->registerToolStrip( m_strip, this, SLOT( driveSelected( int ) ) );
}

bool PaneWidget::eventFilter( QObject* watched, QEvent* e )
{
    if ( watched == m_edit ) {
        switch ( e->type() ) {
            case QEvent::FocusIn:
                return editFocusInEvent( static_cast<QFocusEvent*>( e ) );
            case QEvent::FocusOut:
                return editFocusOutEvent( static_cast<QFocusEvent*>( e ) );
            case QEvent::KeyPress:
                return editKeyPressEvent( static_cast<QKeyEvent*>( e ) );
            default:
                break;
        }
    }

    if ( watched == m_view || watched == m_view->viewport() ) {
        switch ( e->type() ) {
            case QEvent::KeyPress:
                return viewKeyPressEvent( static_cast<QKeyEvent*>( e ) );
            case QEvent::MouseButtonPress:
                return viewMouseButtonPressEvent( static_cast<QMouseEvent*>( e ) );
            case QEvent::MouseMove:
                return viewMouseMoveEvent( static_cast<QMouseEvent*>( e ) );
            case QEvent::MouseButtonRelease:
                return viewMouseButtonReleaseEvent( static_cast<QMouseEvent*>( e ) );
            case QEvent::MouseButtonDblClick:
                return viewMouseDoubleClickEvent( static_cast<QMouseEvent*>( e ) );
            case QEvent::DragEnter:
                return viewDragEnterEvent( static_cast<QDragEnterEvent*>( e ) );
            case QEvent::DragMove:
                return viewDragMoveEvent( static_cast<QDragMoveEvent*>( e ) );
            case QEvent::DragLeave:
                return viewDragLeaveEvent( static_cast<QDragLeaveEvent*>( e ) );
            case QEvent::Drop:
                return viewDropEvent( static_cast<QDropEvent*>( e ) );
            case QEvent::Resize:
                if ( watched == m_view )
                    resizeColumns();
                return false;
            case QEvent::Paint:
                if ( watched == m_view->viewport() )
                    gotoItem();
                return false;
            default:
                break;
        }
    }

    return false;
}

bool PaneWidget::editFocusInEvent( QFocusEvent* /*e*/ )
{
    m_edit->setPalette( palette() );
    return false;
}

bool PaneWidget::editFocusOutEvent( QFocusEvent* /*e*/ )
{
    updateEditPalette();
    updateLocation();
    return false;
}

bool PaneWidget::editKeyPressEvent( QKeyEvent* e )
{
    switch ( e->key() ) {
        case Qt::Key_Escape:
            m_view->setFocus();
            return true;
        default:
            break;
    }
    return false;
}

bool PaneWidget::viewKeyPressEvent( QKeyEvent* e )
{
    switch ( ( e->key() | e->modifiers() ) & ~Qt::KeypadModifier ) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if ( !m_view->isEditing() ) {
                openItem( m_view->currentIndex() );
                return true;
            }
            break;

        case Qt::Key_Right:
            if ( m_model->isParentFolder( m_view->currentIndex() ) )
                openParent();
            else
                enterDirectory( m_model->itemAt( m_view->currentIndex() ) );
            return true;

        case Qt::ControlModifier + Qt::Key_PageDown:
            enterDirectory( m_model->itemAt( m_view->currentIndex() ) );
            return true;

        case Qt::Key_Backspace:
        case Qt::Key_Left:
        case Qt::ControlModifier + Qt::Key_PageUp:
            openParent();
            return true;

        case Qt::Key_Space:
            if ( !m_model->isItemSelected( m_view->currentIndex() ) )
                m_model->calculateSize( m_view->currentIndex() );
            m_model->toggleItemSelected( m_view->currentIndex() );
            return true;

        case Qt::Key_Insert: {
            m_model->toggleItemSelected( m_view->currentIndex() );
            QModelIndex index = m_view->indexBelow( m_view->currentIndex() );
            if ( index.isValid() )
                m_view->setCurrentIndex( index );
            return true;
        }

        case Qt::ShiftModifier + Qt::Key_Up:
        case Qt::ShiftModifier + Qt::Key_Down: {
            m_model->toggleItemSelected( m_view->currentIndex() );
            QModelIndex index = ( e->key() == Qt::Key_Up ) ? m_view->indexAbove( m_view->currentIndex() ) : m_view->indexBelow( m_view->currentIndex() );
            if ( index.isValid() )
                m_view->setCurrentIndex( index );
            return true;
        }

        case Qt::ShiftModifier + Qt::Key_PageUp: {
            QModelIndex fromIndex = m_view->currentIndex();
            if ( fromIndex.isValid() ) {
                QModelIndex toIndex = m_view->movePageUp();
                if ( toIndex.isValid() ) {
                    int toRow = toIndex.row();
                    if ( toRow == 0 )
                        toRow--;
                    for ( int i = fromIndex.row(); i > toRow; i-- )
                        m_model->toggleItemSelected( m_model->index( i, 0 ) );
                    m_view->setCurrentIndex( toIndex );
                }
            }
            return true;
        }

        case Qt::ShiftModifier + Qt::Key_PageDown: {
            QModelIndex fromIndex = m_view->currentIndex();
            if ( fromIndex.isValid() ) {
                QModelIndex toIndex = m_view->movePageDown();
                if ( toIndex.isValid() ) {
                    int toRow = toIndex.row();
                    if ( toRow == m_model->rowCount() - 1 )
                        toRow++;
                    for ( int i = fromIndex.row(); i < toRow; i++ )
                        m_model->toggleItemSelected( m_model->index( i, 0 ) );
                    m_view->setCurrentIndex( toIndex );
                }
            }
            return true;
        }

        case Qt::ShiftModifier + Qt::Key_Home: {
            QModelIndex fromIndex = m_view->currentIndex();
            if ( fromIndex.isValid() ) {
                for ( int i = fromIndex.row(); i >= 0; i-- )
                    m_model->toggleItemSelected( m_model->index( i, 0 ) );
                m_view->setCurrentIndex( fromIndex.sibling( 0, 0 ) );
            }
            return true;
        }

        case Qt::ShiftModifier + Qt::Key_End: {
            QModelIndex fromIndex = m_view->currentIndex();
            if ( fromIndex.isValid() ) {
                for ( int i = fromIndex.row(); i < m_model->rowCount(); i++ )
                    m_model->toggleItemSelected( m_model->index( i, 0 ) );
                m_view->setCurrentIndex( fromIndex.sibling( m_model->rowCount() - 1, 0 ) );
            }
            return true;
        }

        case Qt::ALT + Qt::Key_Left: {
            if ( m_historyIndex < m_history.count() - 1 )
                setHistoryIndex( m_historyIndex + 1 );
            return true;
        }

        case Qt::ALT + Qt::Key_Right: {
            if ( m_historyIndex > 0 )
                setHistoryIndex( m_historyIndex - 1 );
            return true;
        }

        default:
            break;
    }

    return false;
}

bool PaneWidget::viewMouseButtonPressEvent( QMouseEvent* e )
{
    QModelIndex index = m_view->indexAt( e->pos() );

    if ( index.isValid() && !m_model->isParentFolder( index ) )
        m_startDragPosition = e->pos();

    m_renameIndex = QModelIndex();
    m_renameTimer->stop();

    if ( e->button() == Qt::LeftButton ) {
        if ( e->modifiers().testFlag( Qt::ShiftModifier ) ) {
            if ( index.isValid() ) {
                QModelIndex anchor = m_view->anchor();
                if ( !anchor.isValid() )
                    anchor = m_view->currentIndex();
                if ( anchor.isValid() ) {
                    int from = qMin( anchor.row(), index.row() );
                    int to = qMax( anchor.row(), index.row() );
                    if ( e->modifiers().testFlag( Qt::ControlModifier ) ) {
                        bool isSelected = m_model->isItemSelected( index );
                        for ( int i = from; i <= to; i++ )
                            m_model->setItemSelected( m_model->index( i, 0 ), !isSelected );
                    } else {
                        for ( int i = 0; i < m_model->rowCount(); i++ )
                            m_model->setItemSelected( m_model->index( i, 0 ), i >= from && i <= to );
                    }
                    m_view->setCurrentIndex( index );
                    m_view->setAnchor( anchor );
                    return true;
                }
            }
        } else if ( e->modifiers().testFlag( Qt::ControlModifier ) ) {
            if ( index.isValid() ) {
                if ( index != m_view->currentIndex() && m_model->selectedItems().isEmpty() )
                    m_model->setItemSelected( m_view->currentIndex(), true );
                m_model->toggleItemSelected( index );
            }
        } else {
            if ( index.isValid() && index == m_view->currentIndex() )
                m_renameIndex = index;

            if ( !m_model->isItemSelected( index ) )
                m_model->unselectAll();
        }
    }

    if ( e->button() == Qt::RightButton ) {
        if ( !m_model->isItemSelected( index ) )
            m_model->unselectAll();
    }

    if ( e->button() == Qt::XButton1 ) {
        if ( m_historyIndex < m_history.count() - 1 )
            setHistoryIndex( m_historyIndex + 1 );
    }

    if ( e->button() == Qt::XButton2 ) {
        if ( m_historyIndex > 0 )
            setHistoryIndex( m_historyIndex - 1 );
    }

    m_view->setAnchor( QModelIndex() );

    return false;
}

bool PaneWidget::viewMouseMoveEvent( QMouseEvent* e )
{
    if ( !m_startDragPosition.isNull() && ( m_startDragPosition - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
        m_startDragPosition = QPoint();

        QList<ShellItem> items = m_model->selectedItems();

        if ( items.isEmpty() ) {
            ShellItem item = m_model->itemAt( m_view->currentIndex() );
            if ( item.isValid() ) {
                m_model->setItemSelected( m_view->currentIndex(), true );
                items.append( item );
            }
        }

        if ( !items.isEmpty() ) {
            ShellSelection selection( m_model->folder(), items, this );
            selection.doDragDrop();
        }
    }

    return false;
}

bool PaneWidget::viewMouseButtonReleaseEvent( QMouseEvent* e )
{
    m_startDragPosition = QPoint();

    if ( e->button() == Qt::LeftButton ) {
        QModelIndex index = m_view->indexAt( e->pos() );
        if ( index.isValid() && index == m_view->currentIndex() && index == m_renameIndex )
            m_renameTimer->start();

        if ( !e->modifiers().testFlag( Qt::ShiftModifier ) && !e->modifiers().testFlag( Qt::ControlModifier ) )
            m_model->unselectAll();
    }

    return false;
}

bool PaneWidget::viewMouseDoubleClickEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
        openItem( m_view->indexAt( e->pos() ) );

    if ( e->button() == Qt::XButton1 ) {
        if ( m_historyIndex < m_history.count() - 1 )
            setHistoryIndex( m_historyIndex + 1 );
    }

    if ( e->button() == Qt::XButton2 ) {
        if ( m_historyIndex > 0 )
            setHistoryIndex( m_historyIndex - 1 );
    }

    return true;
}

bool PaneWidget::viewDragEnterEvent( QDragEnterEvent* e )
{
    m_dropData = new ShellDropData( e, m_model->folder(), this );

    m_view->setDragging( true );
    e->accept();

    return true;
}

bool PaneWidget::viewDragMoveEvent( QDragMoveEvent* e )
{
    dragDropHelper( e, false );

    m_view->checkAutoScroll( e->pos() );

    return true;
}

bool PaneWidget::viewDragLeaveEvent( QDragLeaveEvent* /*e*/ )
{
    delete m_dropData;
    m_dropData = NULL;

    m_view->setDragging( false );

    return true;
}

bool PaneWidget::viewDropEvent( QDropEvent* e )
{
    dragDropHelper( e, true );

    delete m_dropData;
    m_dropData = NULL;

    m_view->setDragging( false );

    return true;
}

bool PaneWidget::dragDropHelper( QDropEvent* e, bool doDrop )
{
    if ( !m_dropData || !m_dropData->isValid() )
        return false;

    bool result = false;

    QModelIndex index = m_view->indexAt( e->pos() );

    if ( m_model->isParentFolder( index ) ) {
        result = m_dropData->dragToParent( e );
    } else {
        if ( index.isValid() )
            result = m_dropData->dragToItem( e, m_model->itemAt( index ) );

        if ( !result ) {
            result = m_dropData->dragToFolder( e );
            index = QModelIndex();
        }
    }

    if ( !doDrop )
        m_view->highlightDropItem( index );

    if ( result && doDrop ) {
        LocalSettings* settings = application->applicationSettings();

        if ( !( e->mouseButtons() & Qt::RightButton ) && settings->value( "ConfirmDnd" ).toBool() ) {
            if ( m_dropData->dropAction() & ( Qt::CopyAction | Qt::MoveAction ) ) {
                ShellSelection* selection = ShellSelection::draggedSelection( m_dropData );
                if ( selection ) {
                    ShellFolder* targetFolder;
                    if ( m_model->isParentFolder( index ) ) {
                        ShellItem item;
                        targetFolder = folder()->parentFolder( item );
                    } else if ( index.isValid() ) {
                        targetFolder = folder()->openFolder( m_model->itemAt( index ) );
                    } else {
                        targetFolder = folder();
                    }

                    if ( targetFolder ) {
                        mainWindow->transferSelection( selection, targetFolder, m_dropData->dropAction() & Qt::CopyAction ? ShellSelection::Copy : ShellSelection::Move, false );

                        if ( targetFolder != folder() )
                            delete targetFolder;

                        result = false;
                    }
                }
            }

            if ( result ) {
                QString targetPath;
                if ( m_model->isParentFolder( index ) ) {
                    ShellItem item;
                    ShellFolder* parentFolder = folder()->parentFolder( item );
                    if ( parentFolder ) {
                        targetPath = parentFolder->path();
                        delete parentFolder;
                    }
                } else if ( index.isValid() ) {
                    targetPath = folder()->itemPath( m_model->itemAt( index ) );
                } else {
                    targetPath = folder()->path();
                }

                mainWindow->raise();
                mainWindow->activateWindow();

                if ( QMessageBox::question( this, tr( "Drag & Drop" ), tr( "Drop selected items to <b>%1</b>?" ).arg( targetPath ),
                     QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
                    result = false;
            }
        }

        if ( result )
            result = m_dropData->drop();
    }

    if ( result ) {
        e->setDropAction( m_dropData->dropAction() );
        e->accept();
    } else {
        e->ignore();
    }

    return result;
}

void PaneWidget::resizeColumns()
{
    int w = width() - m_view->verticalScrollBar()->sizeHint().width() - m_view->frameWidth() * 2;
    for ( int i = 1; i < m_model->columnCount(); i++ )
        w -= m_view->columnWidth( i );

    bool block = blockSignals( true );
    m_view->setColumnWidth( 0, qMax( w, 150 ) );
    blockSignals( block );
}

void PaneWidget::gotoItem()
{
    QModelIndex index = m_model->gotoItemIndex();
    if ( index.isValid() )
        m_view->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Current );
}

ShellFolder* PaneWidget::folder() const
{
    return m_model->folder();
}

QList<ShellItem> PaneWidget::items() const
{
    return m_model->items();
}

QList<ShellItem> PaneWidget::selectedItems() const
{
    QList<ShellItem> items = m_model->selectedItems();

    if ( items.isEmpty() ) {
        ShellItem item = m_model->itemAt( m_view->currentIndex() );
        if ( item.isValid() )
            items.append( item );
    }

    return items;
}

ShellItem PaneWidget::currentItem() const
{
    return m_model->itemAt( m_view->currentIndex() );
}

void PaneWidget::setGotoItemName( const QString& name )
{
    m_model->setGotoItemName( name );
}

void PaneWidget::refresh()
{
    m_model->refresh();
}

void PaneWidget::viewHidden( bool on )
{
    m_model->setIncludeHidden( on );
    m_model->refresh();
}

void PaneWidget::openDirectory()
{
    m_edit->setFocus( Qt::TabFocusReason );
}

void PaneWidget::changeDirectory()
{
    setDirectory( m_edit->text() );
}

void PaneWidget::setDirectory( const QString& path )
{
    if ( path == m_model->folder()->path() ) {
        m_model->refresh();
        m_view->setFocus();
    } else {
        ShellFolder* folder = new ShellFolder( path, this );
        if ( folder->isValid() ) {
            setFolder( folder );
            activateView();
        } else {
            delete folder;
            QMessageBox::warning( this, tr( "Invalid Path" ), tr( "The path you entered cannot be opened.\nPlease check the spelling and try again." ) );
        }
    }
}

void PaneWidget::openParent()
{
    ShellItem item;
    ShellFolder* folder = m_model->folder()->parentFolder( item );
    if ( folder ) {
        setFolder( folder );
        activateView( item );
    }
}

void PaneWidget::openRoot()
{
    ShellFolder* folder = m_model->folder()->rootFolder();
    if ( folder ) {
        setFolder( folder );
        activateView();
    }
}

void PaneWidget::browse()
{
    ShellFolder* folder = m_model->folder()->browseFolder();
    if ( folder ) {
        setFolder( folder );
        activateView();
    }
}

void PaneWidget::driveSelected( int index )
{
    DriveStripManager* manager = mainWindow->driveStripManager();
    openDrive( manager->driveAt( index ) );
}

void PaneWidget::openDrive( const ShellDrive& drive )
{
    DriveStripManager* manager = mainWindow->driveStripManager();
    ShellFolder* folder = manager->computer()->openRootFolder( drive );
    if ( folder ) {
        setFolder( folder );
        activateView();
    }
}

void PaneWidget::openItem( const QModelIndex& index )
{
    if ( !index.isValid() )
        return;

    if ( m_model->isParentFolder( index ) ) {
        openParent();
        return;
    }

    ShellItem item = m_model->itemAt( index );

    if ( item.attributes().testFlag( ShellItem::Folder ) ) {
        enterDirectory( item );
        return;
    }

    m_model->folder()->executeItem( item );
}

void PaneWidget::enterDirectory( const ShellItem& item )
{
    if ( !item.isValid() )
        return;

    ShellFolder* folder = m_model->folder()->openFolder( item );
    if ( folder ) {
        setFolder( folder );
        activateView();
    }
}

void PaneWidget::setFolder( ShellFolder* folder )
{
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    m_model->setFolder( folder );

    updateLocation();

    if ( !m_lockHistory ) {
        ShellPidl pidl = m_model->folder()->pidl();

        while ( m_historyIndex > 0 ) {
            m_history.removeFirst();
            m_historyIndex--;
        }

        m_history.removeAll( pidl );

        m_history.prepend( pidl );

        while ( m_history.count() > 20 )
            m_history.removeLast();
    }

    QApplication::restoreOverrideCursor();
}

void PaneWidget::activateView( const ShellItem& item /*= ShellItem()*/ )
{
    QModelIndex index;
    if ( item.isValid() )
        index = m_model->indexOf( item );
    if ( !index.isValid() && m_model->rowCount() > 0 )
        index = m_model->index( 0, 0 );

    m_view->setCurrentIndex( index );
    m_view->setFocus();
}

void PaneWidget::setSourcePane( bool source )
{
    if ( m_isSource != source ) {
        m_isSource = source;

        updateEditPalette();

        if ( source && !m_edit->hasFocus() )
            m_view->setFocus();
    }
}

void PaneWidget::updateLocation()
{
    QString path = m_model->folder()->path();
    m_edit->setText( path );
}

void PaneWidget::updateEditPalette()
{
    QPalette palette = m_edit->palette();
    if ( m_isSource ) {
        palette.setColor( QPalette::Base, palette.color( QPalette::Highlight ) );
        palette.setColor( QPalette::Text, palette.color( QPalette::HighlightedText ) );
    } else {
        palette.setColor( QPalette::Base, palette.color( QPalette::Window ) );
        palette.setColor( QPalette::Text, palette.color( QPalette::WindowText ) );
    }
    m_edit->setPalette( palette );
}

void PaneWidget::selectAll()
{
    m_model->selectAll();
}

void PaneWidget::unselectAll()
{
    m_model->unselectAll();
}

void PaneWidget::invertSelection()
{
    m_model->invertSelection();
}

void PaneWidget::setPatternSelection( const QString& pattern, bool selected )
{
    QStringList parts = pattern.split( QLatin1Char( ';' ), QString::SkipEmptyParts );

    QList<QRegExp> filters;
    foreach ( QString part, parts )
        filters.append( QRegExp( part, Qt::CaseInsensitive, QRegExp::Wildcard ) );

    for ( int i = 0; i < m_model->rowCount(); i++ ) {
        QModelIndex index = m_model->index( i, 0 );
        if ( !m_model->isParentFolder( index ) ) {
            foreach ( QRegExp filter, filters ) {
                if ( filter.exactMatch( m_model->itemAt( index ).name() ) ) {
                    m_model->setItemSelected( index, selected );
                    break;
                }
            }
        }
    }
}

void PaneWidget::renameCurrent()
{
    QModelIndex index = m_view->currentIndex();
    ShellItem item = m_model->itemAt( index );

    if ( item.isValid() && item.attributes().testFlag( ShellItem::CanRename ) ) {
        m_view->edit( index );

        QWidget* editor = m_view->indexWidget( index );
        if ( QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ) {
            QString text = lineEdit->text();
            int pos = text.lastIndexOf( '.' );
            if ( pos > 0 )
                lineEdit->setSelection( 0, pos );
        }
    }
}

void PaneWidget::calculateSize()
{
    if ( m_model->selectedItemsCount() > 0 )
        m_model->calculateSizeSelected();
    else
        m_model->calculateSize( m_view->currentIndex() );
}

void PaneWidget::compareWith( const QList<ShellItem>& items )
{
    m_model->compareWith( items );
}

void PaneWidget::showDrivesMenu()
{
    DriveStripManager* manager = mainWindow->driveStripManager();
    manager->showDrivesMenu( m_strip );
}

void PaneWidget::showHistory()
{
    QMenu menu;

    QList<QAction*> actions;
    for ( int i = 0; i < m_history.count(); i++ ) {
        QAction* action = menu.addAction( m_history.at( i ).path().replace( QLatin1String( "&" ), QLatin1String( "&&" ) ) );

        if ( m_historyIndex == i )
            menu.setDefaultAction( action );

        actions.append( action );
    }

    QAction* action = menu.exec( m_historyButton->mapToGlobal( m_historyButton->rect().bottomLeft() ) );

    if ( action ) {
        int index = actions.indexOf( action );
        if ( index >= 0 )
            setHistoryIndex( index );
    }
}

void PaneWidget::setHistoryIndex( int index )
{
    m_lockHistory = true;

    m_historyIndex = index;

    ShellFolder* folder = new ShellFolder( m_history.at( index ), this );
    if ( folder->isValid() ) {
        setFolder( folder );
        activateView();
    } else {
        delete folder;
        QMessageBox::warning( this, tr( "Invalid Path" ), tr( "The path you selected cannot be opened.\nMake sure the device is available and try again." ) );
    }

    m_lockHistory = false;
}

void PaneWidget::showBookmarks()
{
    QMenu menu;

    QList<Bookmark> bookmarks = application->bookmarks();
    qSort( bookmarks );

    QList<QAction*> actions;
    for ( int i = 0; i < bookmarks.count(); i++ ) {
        QAction* action = menu.addAction( bookmarks.at( i ).name().replace( QLatin1String( "&" ), QLatin1String( "&&" ) ) );
        actions.append( action );
    }

    if ( !actions.isEmpty() )
        menu.addSeparator();

    menu.addAction( mainWindow->action( "addBookmark" ) );
    menu.addAction( mainWindow->action( "editBookmarks" ) );

    mainWindow->action( "editBookmarks" )->setEnabled( !actions.isEmpty() );

    QAction* action = menu.exec( m_bookmarkButton->mapToGlobal( m_bookmarkButton->rect().bottomLeft() ) );
    if ( action ) {
        int index = actions.indexOf( action );
        if ( index >= 0 )
            setBookmark( bookmarks.at( index ) );
    }
}

void PaneWidget::setBookmark( const Bookmark& bookmark )
{
    if ( !bookmark.user().isEmpty() ) {
        OpenFtpDialog dialog( this );
        dialog.setPath( bookmark.path() );
        dialog.setUser( bookmark.user() );
        dialog.setPassword( bookmark.password() );

        if ( dialog.exec() != QDialog::Accepted )
            return;

        setDirectory( dialog.path() );
    } else {
        ShellFolder* folder = bookmark.createFolder( this );
        if ( folder->isValid() ) {
            setFolder( folder );
            activateView();
        } else {
            delete folder;
            QMessageBox::warning( this, tr( "Invalid Bookmark" ), tr( "The bookmark you selected cannot be opened.\nMake sure the device is available and try again." ) );
        }
    }
}

void PaneWidget::viewContextMenuRequested( const QPoint& pos )
{
    if ( !m_view->viewport()->visibleRegion().contains( pos ) )
        return;

    QModelIndex index = m_view->indexAt( pos );

    if ( index.isValid() && !m_model->isParentFolder( index ) ) {
        QList<ShellItem> items;

        if ( m_model->isItemSelected( index ) ) {
            items = m_model->selectedItems();
        } else {
            m_model->unselectAll();
            m_model->setItemSelected( index, true );
            items.append( m_model->itemAt( index ) );
        }

        m_view->setCurrentIndex( index );

        ShellSelection::Flags flags = ShellSelection::CanRename;

        if ( items.count() == 1 && items.first().attributes().testFlag( ShellItem::Folder ) )
            flags |= ShellSelection::CanOpen;

        ShellSelection selection( m_model->folder(), items, this );
        ShellSelection::MenuCommand command = selection.showContextMenu( m_view->viewport()->mapToGlobal( pos ), flags );

        if ( command == ShellSelection::Open )
            enterDirectory( items.first() );
        else if ( command == ShellSelection::Rename )
            renameCurrent();
    } else {
        m_model->unselectAll();

        m_model->folder()->showContextMenu( m_view->viewport()->mapToGlobal( pos ), 0 );
    }
}

void PaneWidget::stripContextMenuRequested( const QPoint& pos )
{
    DriveStripManager* manager = mainWindow->driveStripManager();

    ShellDrive drive = manager->driveAt( m_strip, pos );

    if ( drive.isValid() ) {
        ShellSelection::MenuCommand command = manager->computer()->showContextMenu( drive, m_strip->mapToGlobal( pos ), ShellSelection::CanOpen );

        if ( command == ShellSelection::Open )
            openDrive( drive );
    } else {
        QMenu* menu = mainWindow->builder()->contextMenu( "menuDrives" );

        if ( menu )
            menu->popup( m_strip->mapToGlobal( pos ) );
    }
}

void PaneWidget::renameTimeout()
{
    if ( m_renameIndex.isValid() && m_renameIndex == m_view->currentIndex() ) {
        if ( m_renameIndex == m_view->indexAt( m_view->viewport()->mapFromGlobal( QCursor::pos() ) ) )
            renameCurrent();
    }
}

void PaneWidget::sectionResized( int index, int /*oldSize*/, int newSize )
{
    emit headerSectionResized( index, newSize );
}

void PaneWidget::sectionMoved( int /*index*/, int from, int to )
{
    m_movingSection = true;
    emit headerSectionMoved( from, to );
    m_movingSection = false;
}

void PaneWidget::resizeHeaderSection( int index, int size )
{
    m_view->header()->resizeSection( index, size );
}

void PaneWidget::moveHeaderSection( int from, int to )
{
    if ( !m_movingSection )
        m_view->header()->moveSection( from, to );
}

void PaneWidget::updateStatus()
{
    DriveStripManager* manager = mainWindow->driveStripManager();
    if ( !manager )
        return;

    int selectedCount = m_model->selectedItemsCount();
    int totalCount = m_model->totalItemsCount();

    qint64 selectedSize = m_model->selectedItemsSize();
    qint64 totalSize = m_model->totalItemsSize();

    m_selectionStatus->setText( QString( "%1 of %2 items selected (%3 of %4)" ).arg( QString::number( selectedCount ), QString::number( totalCount ),
        formatSize( selectedSize, false ), formatSize( totalSize, true ) ) );

    ShellDrive drive = manager->driveFromFolder( m_model->folder() );

    if ( drive.isValid() ) {
        qint64 free;
        qint64 total;
        if ( drive.getFreeSpace( &free, &total ) && total > 0 ) {
            m_driveStatus->setText( tr( "%1 - %2 of %3 free (%4%)" ).arg( drive.name(), formatSize( free, false ), formatSize( total, true ),
                QString::number( free * 100 / total ) ) );
        } else {
            m_driveStatus->setText( drive.name() );
        }
        m_driveStatus->show();
    } else {
        m_driveStatus->hide();
    }
}

QString PaneWidget::formatSize( qint64 size, bool afterOf )
{
    if ( size == 0 && !afterOf )
        return "0";
    if ( size < 1024 )
        return !afterOf ? tr( "%n bytes", "before of", size ) : tr( "%n bytes", "after of", size );
    if ( size < 1048576 )
        return tr( "%1 kB" ).arg( size / 1024.0, 0, 'f', 1 );
    if ( size < 1073741824 )
        return tr( "%1 MB" ).arg( size / 1048576.0, 0, 'f', 1 );
    return tr( "%1 GB" ).arg( size / 1073741824.0, 0, 'f', 1 );
}
