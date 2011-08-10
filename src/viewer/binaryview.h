/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011 Michał Męciński
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

#ifndef BINARYVIEW_H
#define BINARYVIEW_H

#include "viewer/view.h"

class BinaryView : public View
{
    Q_OBJECT
public:
    BinaryView( QObject* parent, QWidget* parentWidget );
    ~BinaryView();

public: // overrides
    Type type() const;

    void load();

private slots:
    void updateActions();

    void copy();
    void selectAll();

    void toggleHexMode();

    void contextMenuRequested( const QPoint& pos );

private:
    void initializeSettings();
    void storeSettings();

private:
    QPlainTextEdit* m_edit;

    bool m_hexMode;
    bool m_lastHexMode;
};

#endif
