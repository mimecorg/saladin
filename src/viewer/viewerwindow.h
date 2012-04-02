/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2012 Michał Męciński
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

#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include "viewer/view.h"
#include "xmlui/client.h"

#include <QMainWindow>

class ViewerWindow : public QMainWindow, public XmlUi::Client
{
    Q_OBJECT
public:
    ViewerWindow();
    ~ViewerWindow();

public:
    void setView( View* view );

protected: // overrides
    void showEvent( QShowEvent* e );

private slots:
    void reload();

    void switchToText();
    void switchToBinary();
    void switchToImage();

    void statusChanged( const QString& status );

private:
    void initializeGeometry();
    void storeGeometry( bool offset );

private:
    View* m_view;

    QLabel* m_statusLabel;
};

#endif
