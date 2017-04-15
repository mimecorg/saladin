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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "shell/shellpidl.h"

class SeparatorComboBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog( QWidget* parent );
    ~SettingsDialog();

public: // overrides
    void accept();

private slots:
    bool apply();

    void browseLeftPane();
    void browseRightPane();

    void browseViewer();
    void browseEditor();
    void browseConsole();
    void browseDiff();

    void showGeneralTab();
    void showToolsTab();

    void loadIcons();

private:
    void browsePane( QLineEdit* edit, int index );
    void browseTool( QLineEdit* edit );

    void loadLanguages();

    QToolButton* browseButton( QWidget* parent, const char* slot );

private:
    QLabel* m_promptPixmap;

    QStackedWidget* m_stackedWidget;

    QAction* m_generalAction;
    QAction* m_toolsAction;

    QToolButton* m_leftPaneButton;
    QToolButton* m_rightPaneButton;
    QToolButton* m_viewerButton;
    QToolButton* m_editorButton;
    QToolButton* m_consoleButton;
    QToolButton* m_diffButton;

    SeparatorComboBox* m_languageComboBox;

    QLineEdit* m_leftPaneEdit;
    QLineEdit* m_rightPaneEdit;
    QCheckBox* m_rememberCheckBox;

    QComboBox* m_themeComboBox;

    QFontComboBox* m_binaryFontComboBox;
    QSpinBox* m_binaryFontSpinBox;
    QFontComboBox* m_textFontComboBox;
    QSpinBox* m_textFontSpinBox;

    QCheckBox* m_confirmDndCheckBox;

    QCheckBox* m_updateCheckBox;

    QCheckBox* m_internalViewerCheckBox;

    QLineEdit* m_viewerEdit;
    QLineEdit* m_editorEdit;
    QLineEdit* m_consoleEdit;
    QLineEdit* m_diffEdit;

    ShellPidl m_pidls[ 2 ];
};

#endif
