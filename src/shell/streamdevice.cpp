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

#include "streamdevice.h"
#include "streamdevice_p.h"
#include "shellpidl_p.h"

StreamDevice::StreamDevice( const ShellPidl& pidl ) :
    d( new StreamDevicePrivate() )
{
    d->q = this;

    d->m_pidl = pidl;

    SHBindToObject( NULL, pidl.d->pidl(), NULL, IID_PPV_ARGS( &d->m_stream ) );
}

StreamDevice::~StreamDevice()
{
    delete d;
}

StreamDevicePrivate::StreamDevicePrivate() :
    q( NULL ),
    m_stream( NULL )
{
}

StreamDevicePrivate::~StreamDevicePrivate()
{
    if ( m_stream ) {
        m_stream->Release();
        m_stream = NULL;
    }
}

QString StreamDevice::name() const
{
    QString name;

    if ( !d->m_pidl.path().isEmpty() ) {
        QFileInfo info( d->m_pidl.path() );
        name = info.fileName();
    } else if ( d->m_stream ) {
        STATSTG stat = {};
        HRESULT hr = d->m_stream->Stat( &stat, STATFLAG_DEFAULT );

        if ( SUCCEEDED( hr ) ) {
            name = QString::fromWCharArray( stat.pwcsName );

            CoTaskMemFree( stat.pwcsName );
        }
    }

    return name;
}

bool StreamDevice::open( OpenMode mode )
{
    if ( !d->m_stream )
        return false;
    if ( mode & WriteOnly )
        return false;

    return QIODevice::open( mode );
}

qint64 StreamDevice::size() const
{
    if ( d->m_stream ) {
        STATSTG stat = {};
        HRESULT hr = d->m_stream->Stat( &stat, STATFLAG_NONAME );

        if ( SUCCEEDED( hr ) )
            return stat.cbSize.QuadPart;
    }

    return -1;
}

bool StreamDevice::seek( qint64 pos )
{
    if ( d->m_stream ) {
        QIODevice::seek( pos );

        LARGE_INTEGER lipos;
        lipos.QuadPart = pos;

        HRESULT hr = d->m_stream->Seek( lipos, STREAM_SEEK_SET, NULL );

        if ( SUCCEEDED( hr ) )
            return true;
    }

    return false;
}

qint64 StreamDevice::readData( char* data, qint64 maxSize )
{
    if ( d->m_stream ) {
        ULONG read = 0;
        HRESULT hr = d->m_stream->Read( data, maxSize, &read );

        if ( SUCCEEDED( hr ) )
            return read;
    }

    return -1;
}

qint64 StreamDevice::writeData( const char* /*data*/, qint64 /*maxSize*/ )
{
    return -1;
}
