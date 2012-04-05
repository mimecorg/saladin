/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2012 Michał Męciński
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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QSharedData>

class BookmarkPrivate;
class ShellFolder;

class Bookmark
{
public:
    Bookmark();
    Bookmark( const QString& name, ShellFolder* folder, bool withPassword );
    ~Bookmark();

    Bookmark( const Bookmark& other );
    Bookmark& operator =( const Bookmark& other );

public:
    bool isValid() const;

    QString name() const;
    void setName( const QString& name );

    QString path() const;
    QString user() const;
    QString password() const;

    ShellFolder* createFolder( QWidget* parent ) const;

public:
    friend bool operator <( const Bookmark& lhs, const Bookmark& rhs );

    friend QDataStream& operator <<( QDataStream& stream, const Bookmark& bookmark );
    friend QDataStream& operator >>( QDataStream& stream, Bookmark& bookmark );

private:
    QSharedDataPointer<BookmarkPrivate> d;
};

#endif
