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

#ifndef PANEWIDGET_H
#define PANEWIDGET_H

#include "shell/shellitem.h"
#include "xmlui/toolstrip.h"
#include "xmlui/toolstrip_p.h"

#include <QWidget>

class FolderItemView;
class FolderItemModel;
class DriveStripManager;
class ShellFolder;
class ShellDrive;
class ShellPidl;
class ShellDropData;
class Bookmark;

class PaneWidget : public QWidget
{
    Q_OBJECT
public:
    enum PaneLocation
    {
        LeftPane,
        RightPane
    };

public:
    PaneWidget( PaneLocation location, QWidget* parent );
    ~PaneWidget();

public:
    void setSourcePane( bool source );

    void setFolder( ShellFolder* folder );
    ShellFolder* folder() const;

    QList<ShellItem> items() const;
    QList<ShellItem> selectedItems() const;
    ShellItem currentItem() const;

    void activateView( const ShellItem& item = ShellItem() );

    void refresh();
    void viewHidden( bool on );

    void openDirectory();
    void openParent();
    void openRoot();
    void browse();
    void setDirectory( const QString& path );

    void selectAll();
    void unselectAll();
    void invertSelection();
    void setPatternSelection( const QString& pattern, bool selected );

    void renameCurrent();
    void calculateSize();
    void compareWith( const QList<ShellItem>& items );
    void showDrivesMenu();
    void showHistory();
    void showBookmarks();

public: // overrides
    bool eventFilter( QObject* watched, QEvent* e );

public slots:
    void resizeHeaderSection( int index, int size );
    void moveHeaderSection( int from, int to );

signals:
    void headerSectionResized( int index, int size );
    void headerSectionMoved( int from, int to );

private slots:
    void changeDirectory();
    void driveSelected( int index );

    void openItem( const QModelIndex& index );
    void viewContextMenuRequested( const QPoint& pos );

    void stripContextMenuRequested( const QPoint& pos );

    void sectionResized( int index, int oldSize, int newSize );
    void sectionMoved( int index, int from, int to );

    void renameTimeout();

    void updateStatus();

private:
    bool editFocusInEvent( QFocusEvent* e );
    bool editFocusOutEvent( QFocusEvent* e );

    bool editKeyPressEvent( QKeyEvent* e );

    bool viewKeyPressEvent( QKeyEvent* e );

    bool viewMouseButtonPressEvent( QMouseEvent* e );
    bool viewMouseMoveEvent( QMouseEvent* e );
    bool viewMouseButtonReleaseEvent( QMouseEvent* e );
    bool viewMouseDoubleClickEvent( QMouseEvent* e );

    bool viewDragEnterEvent( QDragEnterEvent* e );
    bool viewDragMoveEvent( QDragMoveEvent* e );
    bool viewDragLeaveEvent( QDragLeaveEvent* e );
    bool viewDropEvent( QDropEvent* e );

    bool dragDropHelper( QDropEvent* e, bool doDrop );

    void enterDirectory( const ShellItem& item );
    void openDrive( const ShellDrive& drive );

    void updateLocation();
    void updateEditPalette();

    QString formatSize( qint64 size, bool afterOf );

    void setHistoryIndex( int index );
    void setBookmark( const Bookmark& bookmark );

private:
    PaneLocation m_location;

    XmlUi::ToolStrip* m_strip;

    QLineEdit* m_edit;

    XmlUi::ActionButton* m_bookmarkButton;
    XmlUi::ActionButton* m_historyButton;

    FolderItemView* m_view;
    FolderItemModel* m_model;

    QLabel* m_selectionStatus;
    QLabel* m_driveStatus;

    bool m_isSource;

    bool m_movingSection;

    QPersistentModelIndex m_renameIndex;
    QTimer* m_renameTimer;

    QList<ShellPidl> m_history;
    int m_historyIndex;
    bool m_lockHistory;

    QPoint m_startDragPosition;

    ShellDropData* m_dropData;
};

#endif
