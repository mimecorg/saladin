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

#ifndef STREAMDEVICE_H
#define STREAMDEVICE_H

#include <QIODevice>

class StreamDevicePrivate;
class ShellPidl;

class StreamDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit StreamDevice( const ShellPidl& pidl );
    ~StreamDevice();

public:
    QString name() const;

public: // overrides
    bool open( OpenMode mode );

    qint64 size() const;
    bool seek( qint64 pos );

protected: // overrides
    qint64 readData( char* data, qint64 maxSize );
    qint64 writeData( const char* data, qint64 maxSize );

private:
    StreamDevicePrivate* d;
};

#endif
