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

#ifndef VIEW_H
#define VIEW_H

#include "xmlui/client.h"

#include <QObject>

class ShellPidl;

class View : public QObject, public XmlUi::Client
{
    Q_OBJECT
public:
    explicit View( QObject* parent );
    ~View();

public:
    enum Type
    {
        Auto,
        Text,
        Binary,
        Image
    };

public:
    QWidget* mainWidget() const { return m_mainWidget; }

    static View* createView( Type type, QObject* parent, QWidget* parentWidget );

public:
    virtual Type type() const = 0;

    virtual void load( const QString& path, const QByteArray& format ) = 0;

protected:
    void setMainWidget( QWidget* widget );

private:
    QWidget* m_mainWidget;
};

#endif
