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

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include "xmlui/client.h"

#include <QDialog>

class ShellFolder;
class FolderItemView;
class SearchItemModel;
class SearchProxyModel;
class ElidedLabel;

class SearchDialog : public QDialog, public XmlUi::Client
{
    Q_OBJECT
public:
    SearchDialog( ShellFolder* folder, QWidget* parent );
    ~SearchDialog();

public: // overrides
    void accept();
    void reject();

private slots:
    void textToggled( bool on );

    void folderEntered( const QString& path );
    void updateResults();

    void searchCompleted();

    void contextMenuRequested( const QPoint& pos );

    void itemDoubleClicked( const QModelIndex& index );

    void viewCurrent();
    void viewAll();
    void editCurrent();
    void gotoFile();

    void copyNames();

private:
    ShellFolder* m_folder;

    QLineEdit* m_locationEdit;
    QComboBox* m_patternComboBox;

    QCheckBox* m_textCheckBox;
    QComboBox* m_textComboBox;
    QCheckBox* m_caseCheckBox;

    QDialogButtonBox* m_buttonBox;

    FolderItemView* m_view;

    SearchItemModel* m_model;
    SearchProxyModel* m_proxyModel;

    ElidedLabel* m_searchStatus;
    ElidedLabel* m_itemsStatus;
};

#endif
