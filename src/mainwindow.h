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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "shell/shellselection.h"
#include "xmlui/client.h"

#include <QMainWindow>

class PaneWidget;
class DriveStripManager;
class ViewManager;
class ShellPidl;

class MainWindow : public QMainWindow, public XmlUi::Client
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

public:
    enum Tool
    {
        ViewerTool,
        EditorTool,
        ConsoleTool,
        DiffTool
    };

public:
    DriveStripManager* driveStripManager() const { return m_driveStripManager; }

    ViewManager* viewManager() const { return m_viewManager; }

    void transferSelection( ShellSelection* selection, ShellFolder* targetFolder, ShellSelection::TransferType type, bool canRename );

    void startTool( Tool tool, ShellFolder* folder, const ShellItem& item );
    void startTool( Tool tool, const QString& path, const QString& directory );

    void gotoFile( const ShellPidl& folderPidl, const ShellItem& item );

public: // overrides
    bool eventFilter( QObject* object, QEvent* e );

protected: // overrides
    void closeEvent( QCloseEvent* e );

private slots:
    void configure();

    void paste();
    void cut();
    void copy();
    void copyNames();

    void refresh();
    void refreshDrives();
    void viewHidden( bool on );

    void openDirectory();
    void openParent();
    void openRoot();
    void openHome();

    void viewSameDirectory();
    void swapPanes();
    void copyToRightPane();
    void copyToLeftPane();

    void selectMask();
    void selectAll();
    void unselectMask();
    void unselectAll();
    void invertSelection();

    void renameCurrent();
    void viewCurrent();
    void viewSelected();
    void editCurrent();
    void editNew();
    void copySelected();
    void cloneSelected();
    void moveSelected();
    void renameSelected();
    void createFolder();
    void moveToTrashCan();
    void deleteSelected();
    void openTerminal();

    void packToZip();
    void browse();
    void openFtpSite();
    void calculateSize();
    void compareFiles();
    void compareDirectories();
    void search();
    void explore();

    void showHistory();
    void showBookmarks();
    void addBookmark();
    void editBookmarks();

    void showDrivesMenu1();
    void showDrivesMenu2();
    void showProperties();
    void otherOpenFolder();
    void otherOpenParent();

    void hideToolStrip();
    void showToolStrip();

    void loadIcons();

private:
    void initializeSettings();
    void saveSettings();

    void restoreDirectory( int index, const QString& key );

    void setSourcePane( int index );

    void invokeCommand( ShellFolder* folder, const QList<ShellItem>& items, const char* verb );

    void transferItems( ShellFolder* sourceFolder, const QList<ShellItem>& items, ShellFolder* targetFolder, ShellSelection::TransferType type );
    void deleteItems( ShellFolder* folder, const QList<ShellItem>& items, ShellSelection::Flags flags );

    QString toolPath( Tool tool );
    QString toolName( Tool tool );

    void createToolStrip();
    void createMenuBar();

private:
    DriveStripManager* m_driveStripManager;

    ViewManager* m_viewManager;

    PaneWidget* m_panes[ 2 ];

    PaneWidget* m_sourcePane;
    PaneWidget* m_targetPane;
};

extern MainWindow* mainWindow;

#endif
