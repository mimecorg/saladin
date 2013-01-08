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

#ifndef SEARCHHELPER_H
#define SEARCHHELPER_H

#include <QThread>

class SearchHelper : public QThread
{
    Q_OBJECT
public:
    SearchHelper( const QString& pattern, Qt::CaseSensitivity cs );
    ~SearchHelper();

public:
    void search( const QStringList& files );

    QList<int> results();

    void abort();

signals:
    void completed();

protected: // overrides
    void run();

private:
    QStringMatcher m_matcher;

    QMutex m_mutex;

    QWaitCondition m_condition;

    QStringList m_files;

    int m_index;
    QList<int> m_results;

    bool m_aborted;
};

#endif
