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

#ifndef MULTIRENAMEWIDGET_H
#define MULTIRENAMEWIDGET_H

#include <QWidget>

class MultiRenameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MultiRenameWidget( QWidget* parent );
    ~MultiRenameWidget();

public:
    void setInputNames( const QStringList& names );

    const QStringList& outputNames() const { return m_outputNames; }

private slots:
    void transformNames();

private:
    QStringList m_inputNames;
    QStringList m_outputNames;

    QLineEdit* m_nameEdit;
    QLineEdit* m_extensionEdit;
    QLineEdit* m_searchEdit;
    QLineEdit* m_replaceEdit;

    QCheckBox* m_regExpCheckBox;
    QCheckBox* m_caseCheckBox;
    QComboBox* m_caseComboBox;

    QTreeWidget* m_list;
};

#endif
