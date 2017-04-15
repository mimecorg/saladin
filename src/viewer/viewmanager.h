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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include "viewer/view.h"

#include <QObject>

class ShellPidl;
class ViewManagerPrivate;
class ViewerWindow;
class ViewItem;

class ViewManager : public QObject
{
    Q_OBJECT
public:
    ViewManager();
    ~ViewManager();

public:
    void openView( const ShellPidl& pidl );
    void openView( const QList<ShellPidl>& pidls );

    void loadPrevious( ViewerWindow* window );
    void loadNext( ViewerWindow* window );

    void switchViewType( ViewerWindow* window, View::Type type );

private slots:
    void windowDestroyed( QObject* window );

private:
    void loadView( ViewItem& item, View::Type type );

    void updateTitle( ViewItem& item, const QString& name );

    bool checkType( const ShellPidl& pidl, View::Type inType, View::Type& outType, QByteArray& format, QString& name );

private:
    ViewManagerPrivate* d;
};

#endif
