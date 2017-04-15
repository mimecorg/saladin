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

#ifndef DATASERIALIZER_H
#define DATASERIALIZER_H

#include <QFile>
#include <QDataStream>

class DataSerializer
{
public:
    DataSerializer( const QString& path );
    ~DataSerializer();

public:
    bool openForReading();
    bool openForWriting();

    QDataStream& stream() { return m_stream; }

    static int dataVersion() { return m_dataVersion; }

private:
    QFile m_file;
    QDataStream m_stream;

    static int m_dataVersion;
};

#endif
