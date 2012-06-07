/****************************************************************************
* Simple XML-based UI builder for Qt4
* Copyright (C) 2007-2012 Michał Męciński
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in the
*      documentation and/or other materials provided with the distribution.
*   3. Neither the name of the copyright holder nor the names of the
*      contributors may be used to endorse or promote products derived from
*      this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "client.h"
#include "builder.h"

#include <QDomDocument>
#include <QFile>
#include <QAction>

using namespace XmlUi;

Client::Client() :
    m_builder( NULL )
{
}

Client::~Client()
{
    if ( m_builder )
        m_builder->removeClient( this );
}

void Client::setAction( const QString& id, QAction* action )
{
    m_actions.insert( id, action );
}

QAction* Client::action( const QString& id ) const
{
    return m_actions.value( id, NULL );
}

QList<QAction*> Client::actions() const
{
    return m_actions.values();
}

void Client::setTitle( const QString& id, const QString& title )
{
    m_titles.insert( id, title );
}

QString Client::title( const QString& id ) const
{
    return m_titles.value( id, QString() );
}

void Client::setPopupMenu( const QString& actionId, const QString& menuId, const QString& defaultId )
{
    m_menus.insert( actionId, menuId );
    m_defaultActions.insert( menuId, defaultId );

    QObject::connect( action( actionId ), SIGNAL( triggered() ), action( defaultId ), SIGNAL( triggered() ) );
}

QString Client::popupMenu( const QString& actionId )
{
    return m_menus.value( actionId, QString() );
}

void Client::setDefaultMenuAction( const QString& menuId, const QString& defaultId )
{
    m_defaultActions.insert( menuId, defaultId );
}

QString Client::defaultMenuAction( const QString& menuId )
{
    return m_defaultActions.value( menuId, QString() );
}

bool Client::loadXmlUiFile( const QString& path )
{
    QDomDocument document;

    QFile file( path );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        qWarning( "XmlUi::Client::loadXmlUiFile: Cannot open file" );
        return false;
    }

    QString error;
    int line;
    int column;
    if ( !document.setContent( &file, false, &error, &line, &column ) ) {
        qWarning( "XmlUi::Client::loadXmlUiFile: Cannot parse file" );
        qWarning( "(%s, line: %d, column: %d)", qPrintable( error ), line, column );
        return false;
    }

    m_rootNode = createNode( document.documentElement() );

    if ( m_rootNode.type() != Root ) {
        qWarning( "XmlUi::Client::loadXmlUiFile: Incorrect root element type" );
        return false;
    }

    return true;
}

Node Client::createNode( const QDomElement& element )
{
    Node node;

    node.setType( typeFromTag( element.tagName() ) );
    node.setId( element.attribute( "id", QString() ) );

    for ( QDomElement child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement() )
        node.addChild( createNode( child ) );

    return node;
}

NodeType Client::typeFromTag( const QString& tag )
{
    if ( tag == QLatin1String( "xmlui" ) )
        return Root;
    if ( tag == QLatin1String( "strip" ) )
        return Strip;
    if ( tag == QLatin1String( "section" ) )
        return Section;
    if ( tag == QLatin1String( "grid" ) )
        return Grid;
    if ( tag == QLatin1String( "row" ) )
        return Row;
    if ( tag == QLatin1String( "menu" ) )
        return Menu;
    if ( tag == QLatin1String( "group" ) )
        return Group;
    if ( tag == QLatin1String( "action" ) )
        return Action;
    if ( tag == QLatin1String( "separator" ) )
        return Separator;
    if ( tag == QLatin1String( "merge" ) )
        return Merge;

    qWarning( "XmlUi::Client::loadXmlUiFile: Invalid tag '%s'", qPrintable( tag ) );
    return Unknown;
}
