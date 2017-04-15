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

#include "bookmark.h"
#include "bookmark_p.h"

#include "shell/shellfolder.h"

BookmarkPrivate::BookmarkPrivate() :
    m_type( NullBookmark )
{
}

BookmarkPrivate::BookmarkPrivate( const BookmarkPrivate& other ) : QSharedData( other ),
    m_name( other.m_name ),
    m_type( other.m_type ),
    m_pidl( other.m_pidl ),
    m_path( other.m_path ),
    m_user( other.m_user ),
    m_password( other.m_password )
{
}

BookmarkPrivate::~BookmarkPrivate()
{
}

Bookmark::Bookmark() :
    d( new BookmarkPrivate() )
{
}

Bookmark::Bookmark( const QString& name, ShellFolder* folder, bool withPassword ) :
    d( new BookmarkPrivate() )
{
    d->m_name = name;

    QString path = folder->path();

    if ( path.startsWith( QLatin1String( "ftp://" ), Qt::CaseInsensitive ) ) {
        d->m_type = BookmarkPrivate::FtpBookmark;
        d->m_path = path;
        d->m_user = folder->user();
        if ( withPassword )
            d->m_password = folder->password();
    } else {
        d->m_type = BookmarkPrivate::PidlBookmark;
        d->m_pidl = folder->pidl();
    }
}

Bookmark::~Bookmark()
{
}

Bookmark::Bookmark( const Bookmark& other ) :
    d( other.d )
{
}

Bookmark& Bookmark::operator =( const Bookmark& other )
{
    d = other.d;
    return *this;
}

bool Bookmark::isValid() const
{
    return d->m_type != BookmarkPrivate::NullBookmark;
}

QString Bookmark::name() const
{
    return d->m_name;
}

void Bookmark::setName( const QString& name )
{
    d->m_name = name;
}

QString Bookmark::path() const
{
    switch ( d->m_type ) {
        case BookmarkPrivate::PidlBookmark:
            return d->m_pidl.path();

        case BookmarkPrivate::FtpBookmark:
            return d->m_path;

        default:
            return QString();
    }
}

QString Bookmark::user() const
{
    return d->m_user;
}

QString Bookmark::password() const
{
    return d->m_password;
}

ShellFolder* Bookmark::createFolder( QWidget* parent ) const
{
    switch ( d->m_type ) {
        case BookmarkPrivate::PidlBookmark:
            return new ShellFolder( d->m_pidl, parent );

        case BookmarkPrivate::FtpBookmark: {
            QUrl url( d->m_path );
            url.setUserName( d->m_user );
            url.setPassword( d->m_password );
            return new ShellFolder( url.toString(), parent );
        }

        default:
            return NULL;
    }
}

static QByteArray encrypt( const QString& value )
{
    if ( value.isEmpty() )
        return QByteArray();

    QByteArray data = value.toUtf8();

    DATA_BLOB input;
    input.pbData = (BYTE*)data.data();
    input.cbData = data.length();

    DATA_BLOB output;

    if ( !CryptProtectData( &input, L"", NULL, NULL, NULL, 0, &output ) )
        return QByteArray();

    QByteArray result( (char*)output.pbData, output.cbData );

    LocalFree( output.pbData );

    return result;
}

static QString decrypt( const QByteArray& value )
{
    if ( value.isEmpty() )
        return QString();

    DATA_BLOB input;
    input.pbData = (BYTE*)value.data();
    input.cbData = value.length();

    DATA_BLOB output;

    if ( !CryptUnprotectData( &input, NULL, NULL, NULL, NULL, 0, &output ) )
        return QString();

    QByteArray data( (char*)output.pbData, output.cbData );

    LocalFree( output.pbData );

    return QString::fromUtf8( data );
}

bool operator <( const Bookmark& lhs, const Bookmark& rhs )
{
    return QString::localeAwareCompare( lhs.name(), rhs.name() ) < 0;
}

QDataStream& operator <<( QDataStream& stream, const Bookmark& bookmark )
{
    stream << bookmark.d->m_name << (qint32)bookmark.d->m_type;

    switch ( bookmark.d->m_type ) {
        case BookmarkPrivate::PidlBookmark:
            stream << bookmark.d->m_pidl;
            break;

        case BookmarkPrivate::FtpBookmark:
            stream << bookmark.d->m_path << bookmark.d->m_user << encrypt( bookmark.d->m_password );
            break;
    }

    return stream;
}

QDataStream& operator >>( QDataStream& stream, Bookmark& bookmark )
{
    qint32 type;
    stream >> bookmark.d->m_name >> type;

    bookmark.d->m_type = (BookmarkPrivate::BookmarkType)type;

    switch ( bookmark.d->m_type ) {
        case BookmarkPrivate::PidlBookmark:
            stream >> bookmark.d->m_pidl;
            break;

        case BookmarkPrivate::FtpBookmark: {
            QByteArray password;
            stream >> bookmark.d->m_path >> bookmark.d->m_user >> password;
            bookmark.d->m_password = decrypt( password );
            break;
        }
    }

    return stream;
}
