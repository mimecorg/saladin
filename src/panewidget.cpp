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

#include "panewidget.h"
#include "application.h"
#include "mainwindow.h"
#include "folderitemview.h"
#include "folderitemmodel.h"
#include "folderitemdelegate.h"
#include "drivestripmanager.h"

#include "shell/shellfolder.h"
#include "shell/shellitem.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"

PaneWidget::PaneWidget( PaneLocation location, QWidget* parent ) : QWidget( parent ),
    m_location( location ),
    m_strip( NULL ),
    m_view( NULL ),
    m_model( NULL ),
    m_isSource( false ),
    m_movingSection( false )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setSpacing( 0 );
    layout->setMargin( 0 );

    m_strip = new XmlUi::ToolStrip( this );
    layout->addWidget( m_strip );
    layout->addSpacing( 3 );

    m_strip->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_strip, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( stripContextMenuRequested( const QPoint& ) ) );

    MainWindow* mainWindow = application->mainWindow();

    m_strip->addAuxiliaryAction( mainWindow->action( "openRoot" ) );
    m_strip->addAuxiliaryAction( mainWindow->action( "openParent" ) );
    if ( location == LeftPane )
        m_strip->addAuxiliaryAction( mainWindow->action( "copyToRightPane" ) );
    else
        m_strip->addAuxiliaryAction( mainWindow->action( "copyToLeftPane" ) );

    mainWindow->driveStripManager()->registerToolStrip( m_strip, this, SLOT( driveSelected( int ) ) );

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
    innerLayout->addWidget( m_edit );

    connect( m_edit, SIGNAL( returnPressed() ), this, SLOT( changeDirectory() ) );

    m_edit->installEventFilter( this );

    QToolButton* historyButton = new QToolButton( parent );
    historyButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
    historyButton->setIcon( IconLoader::icon( "history" ) );
    historyButton->setIconSize( QSize( 16, 16 ) );
    historyButton->setToolTip( tr ( "History" ) );
    historyButton->setAutoRaise( true );
    historyButton->setFocusPolicy( Qt::NoFocus );
    editLayout->addWidget( historyButton );

    QToolButton* bookmarkButton = new QToolButton( parent );
    bookmarkButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
    bookmarkButton->setIcon( IconLoader::icon( "bookmark" ) );
    bookmarkButton->setIconSize( QSize( 16, 16 ) );
    bookmarkButton->setToolTip( tr( "Bookmarks" ) );
    bookmarkButton->setAutoRaise( true );
    bookmarkButton->setFocusPolicy( Qt::NoFocus );
    editLayout->addWidget( bookmarkButton );

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
    layout->addWidget( m_view, 1 );

    connect( m_view, SIGNAL( activated( const QModelIndex& ) ), this, SLOT( itemActivated( const QModelIndex& ) ) );
    connect( m_view, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( viewContextMenuRequested( const QPoint& ) ) );

    connect( m_view->header(), SIGNAL( sectionResized( int, int, int ) ), this, SLOT( sectionResized( int, int, int ) ) );
    connect( m_view->header(), SIGNAL( sectionMoved( int, int, int ) ), this, SLOT( sectionMoved( int, int, int ) ) );

    m_view->installEventFilter( this );
    m_view->viewport()->installEventFilter( this );

    m_model = new FolderItemModel( this );
    m_view->setModel( m_model );

    FolderItemDelegate* itemDelegate = new FolderItemDelegate( this );
    m_view->setItemDelegate( itemDelegate );

    m_view->setSortingEnabled( true );
    m_view->sortByColumn( 0, Qt::AscendingOrder );

    LocalSettings* settings = application->applicationSettings();
    QString key = QString( "HeaderState%1" ).arg( location + 1 );
    if ( settings->contains( key ) ) {
        m_view->header()->restoreState( settings->value( key ).toByteArray() );
    } else {
        m_view->setColumnWidth( 0, 210 );
        m_view->setColumnWidth( 1, 90 );
        m_view->setColumnWidth( 2, 110 );
        m_view->setColumnWidth( 3, 70 );
    }

    QStatusBar* status = new QStatusBar( this );
    status->setSizeGripEnabled( false );
    layout->addWidget( status );

    setFocusProxy( m_view );

    updateEditPalette();

    setFolder( new ShellFolder( QDir::toNativeSeparators( QDir::rootPath() ), this ) );
}

PaneWidget::~PaneWidget()
{
    LocalSettings* settings = application->applicationSettings();
    QString key = QString( "HeaderState%1" ).arg( m_location + 1 );
    settings->setValue( key, m_view->header()->saveState() );
}

bool PaneWidget::eventFilter( QObject* watched, QEvent* e )
{
    if ( watched == m_view && e->type() == QEvent::KeyPress ) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>( e );
        switch ( ( keyEvent->key() | keyEvent->modifiers() ) & ~Qt::KeypadModifier ) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if ( !m_view->isEditing() ) {
                    itemActivated( m_view->currentIndex() );
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
                QModelIndex index = ( keyEvent->key() == Qt::Key_Up ) ? m_view->indexAbove( m_view->currentIndex() ) : m_view->indexBelow( m_view->currentIndex() );
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
        }
    }

    if ( m_view && watched == m_view->viewport() && e->type() == QEvent::MouseButtonPress ) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( e );
        if ( mouseEvent->button() == Qt::LeftButton ) {
            if ( mouseEvent->modifiers().testFlag( Qt::ShiftModifier ) ) {
                QModelIndex index = m_view->indexAt( mouseEvent->pos() );
                if ( index.isValid() ) {
                    QModelIndex anchor = m_view->anchor();
                    if ( !anchor.isValid() )
                        anchor = m_view->currentIndex();
                    if ( anchor.isValid() ) {
                        int from = qMin( anchor.row(), index.row() );
                        int to = qMax( anchor.row(), index.row() );
                        if ( mouseEvent->modifiers().testFlag( Qt::ControlModifier ) ) {
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
            } else if ( mouseEvent->modifiers().testFlag( Qt::ControlModifier ) ) {
                QModelIndex index = m_view->indexAt( mouseEvent->pos() );
                if ( index.isValid() ) {
                    if ( index != m_view->currentIndex() && m_model->selectedItems().isEmpty() )
                        m_model->setItemSelected( m_view->currentIndex(), true );
                    m_model->toggleItemSelected( index );
                }
            } else {
                m_model->unselectAll();
            }
        }
        m_view->setAnchor( QModelIndex() );
    }

    if ( watched == m_edit ) {
        if ( e->type() == QEvent::FocusIn ) {
            m_edit->setPalette( palette() );
        } else if ( e->type() == QEvent::FocusOut ) {
            updateEditPalette();
            updateLocation();
        } else if ( e->type() == QEvent::KeyPress ) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>( e );
            if ( keyEvent->key() == Qt::Key_Escape ) {
                m_view->setFocus();
                return true;
            }
        }
    }

    return false;
}

ShellFolder* PaneWidget::folder() const
{
    return m_model->folder();
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
    DriveStripManager* manager = application->mainWindow()->driveStripManager();
    openDrive( manager->driveAt( index ) );
}

void PaneWidget::openDrive( const ShellDrive& drive )
{
    DriveStripManager* manager = application->mainWindow()->driveStripManager();
    ShellFolder* folder = manager->computer()->openRootFolder( drive );
    if ( folder ) {
        setFolder( folder );
        activateView();
    }
}

void PaneWidget::itemActivated( const QModelIndex& index )
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

    if ( path.startsWith( QLatin1String( "ftp://" ) ) ) {
        QUrl url = QUrl( path );
        path = url.toString( QUrl::RemoveUserInfo );
    }

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
    QRegExp regExp( pattern, Qt::CaseInsensitive, QRegExp::Wildcard );

    for ( int i = 0; i < m_model->rowCount(); i++ ) {
        QModelIndex index = m_model->index( i, 0 );
        if ( !m_model->isParentFolder( index ) && regExp.exactMatch( m_model->itemAt( index ).name() ) )
            m_model->setItemSelected( index, selected );
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

void PaneWidget::viewContextMenuRequested( const QPoint& pos )
{
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

        ShellSelection::Flags flags = 0;

        if ( m_model->isParentFolder( index ) )
            flags |= ShellSelection::CanOpen;

        ShellSelection::MenuCommand command = m_model->folder()->showContextMenu( m_view->viewport()->mapToGlobal( pos ), flags );

        if ( command == ShellSelection::Open )
            openParent();
    }
}

void PaneWidget::stripContextMenuRequested( const QPoint& pos )
{
    DriveStripManager* manager = application->mainWindow()->driveStripManager();

    ShellDrive drive = manager->driveAt( m_strip, pos );

    if ( drive.isValid() ) {
        ShellSelection::MenuCommand command = manager->computer()->showContextMenu( drive, m_strip->mapToGlobal( pos ), ShellSelection::CanOpen );

        if ( command == ShellSelection::Open )
            openDrive( drive );
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
