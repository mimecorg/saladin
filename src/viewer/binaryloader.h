/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2013 Michał Męciński
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

#ifndef BINARYLOADER_H
#define BINARYLOADER_H

#include <QThread>

class BinaryLoader : public QThread
{
    Q_OBJECT
public:
    BinaryLoader( const QString& path, bool hexMode );
    ~BinaryLoader();

public:
    QString nextBlock();

    bool atEnd() const { return m_atEnd; }

    qint64 size() const { return m_size; }

    qint64 estimatedLength() const { return m_estimatedLength; }

    void abort();

signals:
    void nextBlockAvailable();

protected: // overrides
    void run();

private:
    QString m_path;
    bool m_hexMode;

    QMutex m_mutex;

    QQueue<QString> m_queue;

    bool m_aborted;
    bool m_signal;

    bool m_atEnd;
    qint64 m_size;
    qint64 m_estimatedLength;
};

#endif
