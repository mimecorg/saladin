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

#ifndef XMLUI_NODE_H
#define XMLUI_NODE_H

#include <QString>
#include <QList>

namespace XmlUi
{

enum NodeType
{
    Unknown,
    Root,
    Strip,
    Section,
    Grid,
    Row,
    Menu,
    Group,
    Action,
    Separator,
    Merge
};

class Node
{
public:
    Node() :
        m_type( Unknown )
    {
    }

    Node( NodeType type ) :
        m_type( type )
    {
    }

public:
    void setType( NodeType type ) { m_type = type; }

    NodeType type() const { return m_type; }

    void setId( const QString& id ) { m_id = id; }

    const QString& id() const { return m_id; }

    void setChildren( const QList<Node>& children ) { m_children = children; }

    const QList<Node>& children() const { return m_children; }

    void addChild( const Node& node ) { m_children.append( node ); }

private:
    NodeType m_type;
    QString m_id;

    QList<Node> m_children;
};

}

#endif
