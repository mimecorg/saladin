/****************************************************************************
* Simple XML-based UI builder for Qt4
* Copyright (C) 2007-2011 Michał Męciński
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

#include "builder.h"
#include "client.h"
#include "toolstrip.h"

#include <QMenu>

using namespace XmlUi;

Builder::Builder( QWidget* parent ) : QObject( parent ),
    m_parent( parent ),
    m_updateEnabled( true ),
    m_updatePending( false )
{
}

Builder::~Builder()
{
    foreach ( Client* client, m_clients )
        client->m_builder = NULL;
}

void Builder::addClient( Client* client )
{
    if ( client->m_builder == this )
        return;

    if ( client->m_builder )
        client->m_builder->removeClient( client );

    m_clients.append( client );
    client->m_builder = this;

    rebuildAll();
}

void Builder::removeClient( Client* client )
{
    if ( client->m_builder != this )
        return;

    m_clients.removeAt( m_clients.indexOf( client ) );
    client->m_builder = NULL;

    rebuildAll();
}

Node Builder::mergeNodes( const Node& node1, const Node& node2 )
{
    Node result;

    result.setType( node1.type() );
    result.setId( node1.id() );

    QList<Node> children;

    int mergePos = -1;

    // copy nodes from node1 and find the position of the merge marker
    foreach ( Node child, node1.children() ) {
        if ( child.type() == Merge )
            mergePos = children.count();
        else
            children.append( child );
    }

    // no merge marker, append node2 at the end
    if ( mergePos < 0 )
        mergePos = children.count();

    int newMergePos = -1;

    // process nodes from node2
    foreach ( Node child, node2.children() ) {
        // remember the position of the merge marker
        if ( child.type() == Merge ) {
            newMergePos = mergePos;
            continue;
        }

        bool merged = false;

        // find a matching node from node1
        for ( int j = 0; j < children.count(); j++ ) {
            if ( canMergeNodes( children.at( j ), child ) ) {
                // replace the original node with the recursively merged node
                children.replace( j, mergeNodes( children.at( j ), child ) );
                merged = true;
                break;
            }
        }

        // no matching node to merge with, insert at merge location
        if ( !merged )
            children.insert( mergePos++, child );
    }

    if ( !children.isEmpty() ) {
        // no merge marker in node2, continue merging from current location
        if ( newMergePos < 0 )
            newMergePos = mergePos;

        // insert new merge marker at appropriate position if needed
        if ( newMergePos < children.count() )
            children.insert( newMergePos, Node( Merge ) );

        result.setChildren( children );
    }

    return result;
}

bool Builder::canMergeNodes( const Node& node1, const Node& node2 )
{
    if ( node1.type() == node2.type() ) {
        // nodes without id are never merged
        if ( node1.id().isEmpty() || node2.id().isEmpty() )
            return false;
        // merge if both type and id is the same
        if ( node1.id() == node2.id() )
            return true;
    }
    return false;
}

Node Builder::resolveGroups( const Node& node )
{
    Node result;

    result.setType( node.type() );
    result.setId( node.id() );

    for ( int i = 0; i < node.children().count(); i++ ) {
        Node child = node.children().at( i );

        // process the element's groups recursively
        if ( child.children().count() > 0 )
            child = resolveGroups( child );

        if ( child.type() == Group ) {
            // replace the group whith its children
            for ( int k = 0; k < child.children().count(); k++ )
                result.addChild( child.children().at( k ) );
        } else {
            result.addChild( child );
        }
    }

    return result;
}

void Builder::supressUpdate()
{
    m_updateEnabled = false;
}

void Builder::resumeUpdate()
{
    m_updateEnabled = true;
    if ( m_updatePending ) {
        rebuildAll();
        m_updatePending = false;
    }
}

void Builder::rebuildAll()
{
    if ( !m_updateEnabled ) {
        m_updatePending = true;
        return;
    }

    QList<QAction*> actionsToAdd;
    QList<QAction*> actionsToRemove = m_parent->actions();

    foreach ( Client* client, m_clients ) {
        foreach ( QAction* action, client->actions() ) {
            if ( !actionsToRemove.removeAll( action ) && !actionsToAdd.contains( action ) )
                actionsToAdd.append( action );
        }
    }

    foreach ( QAction* action, actionsToRemove )
        m_parent->removeAction( action );

    m_parent->addActions( actionsToAdd );

    if ( m_clients.isEmpty() )
        return;

    Node node = m_clients.at( 0 )->m_rootNode;

    for ( int i = 1; i < m_clients.count(); i++ )
        node = mergeNodes( node, m_clients.at( i )->m_rootNode );

    m_rootNode = resolveGroups( node );

    foreach ( QMenu* menu, m_contextMenus )
        menu->deleteLater();
    m_contextMenus.clear();

    foreach ( Node child, m_rootNode.children() ) {
        if ( child.type() == Strip )
            populateToolStrip( child );
    }

    emit reset();
}

void Builder::registerToolStrip( const QString& id, ToolStrip* strip )
{
    m_toolStrips.insert( id, strip );

    foreach ( Node child, m_rootNode.children() ) {
        if ( child.type() == Strip && child.id() == id ) {
            populateToolStrip( child );
            break;
        }
    }
}

void Builder::populateToolStrip( const Node& node )
{
    ToolStrip* strip = m_toolStrips.value( node.id() );
    if ( !strip )
        return;

    strip->clearToolActions();

    bool separator = false;
    bool added = false;

    foreach ( Node child, node.children() ) {
        if ( child.type() == Section ) {
            bool section = false;

            foreach ( Node sectionChild, child.children() ) {
                if ( sectionChild.type() == Grid ) {
                    bool grid = false;

                    foreach ( Node gridChild, sectionChild.children() ) {
                        if ( gridChild.type() == Row ) {
                            bool row = false;

                            foreach ( Node rowChild, gridChild.children() ) {
                                if ( rowChild.type() == Action ) {
                                    QAction* action = findAction( rowChild.id() );

                                    if ( action && action->isVisible() ) {
                                        if ( !section ) {
                                            QString title = findTitle( child.id() );
                                            strip->beginSection( title );
                                            section = true;
                                        }
                                        if ( !grid ) {
                                            strip->beginGrid();
                                            grid = true;
                                        }
                                        if ( !row ) {
                                            strip->beginRow();
                                            row = true;
                                        }
                                        addToolAction( strip, action, rowChild.id() );
                                    }
                                }
                            }

                            if ( row )
                                strip->endRow();
                        }

                        if ( gridChild.type() == Action ) {
                            QAction* action = findAction( gridChild.id() );

                            if ( action && action->isVisible() ) {
                                if ( !section ) {
                                    QString title = findTitle( child.id() );
                                    strip->beginSection( title );
                                    section = true;
                                }
                                if ( !grid ) {
                                    strip->beginGrid();
                                    grid = true;
                                }
                                addToolAction( strip, action, gridChild.id() );
                            }
                        }
                    }

                    if ( grid )
                        strip->endGrid();
                }

                if ( sectionChild.type() == Action ) {
                    QAction* action = findAction( sectionChild.id() );

                    if ( action && action->isVisible() ) {
                        if ( !section ) {
                            QString title = findTitle( child.id() );
                            strip->beginSection( title );
                            section = true;
                        }
                        addToolAction( strip, action, sectionChild.id() );
                    }
                }
            }

            if ( section )
                strip->endSection();
        }

        if ( child.type() == Action ) {
            QAction* action = findAction( child.id() );

            if ( action && action->isVisible() ) {
                if ( separator ) {
                    strip->addSeparator();
                    separator = false;
                }

                addToolAction( strip, action, child.id() );

                added = true;
            }
        }

        if ( child.type() == Separator ) {
            if ( added )
                separator = true;
        }
    }
}

void Builder::addToolAction( ToolStrip* strip, QAction* action, const QString& id )
{
    for ( int i = m_clients.count() - 1; i >= 0; i-- ) {
        QString menuId = m_clients.at( i )->popupMenu( id );
        if ( !menuId.isEmpty() ) {
            QMenu* menu = contextMenu( menuId );
            action->setMenu( menu );
            QString defaultId = m_clients.at( i )->defaultMenuAction( menuId );
            if ( !defaultId.isEmpty() )
                menu->setDefaultAction( findAction( defaultId ) );
            break;
        }
    }

    strip->addToolAction( action );
}

QMenu* Builder::contextMenu( const QString& id )
{
    QMenu* menu = m_contextMenus.value( id );
    if ( menu )
        return menu;

    foreach ( Node child, m_rootNode.children() ) {
        if ( child.type() == Menu && child.id() == id ) {
            menu = createMenu( child );
            break;
        }
    }

    if ( menu ) {
        for ( int i = m_clients.count() - 1; i >= 0; i-- ) {
            QString defaultId = m_clients.at( i )->defaultMenuAction( id );
            if ( !defaultId.isEmpty() ) {
                menu->setDefaultAction( findAction( defaultId ) );
                break;
            }
        }

        m_contextMenus.insert( id, menu );
    }

    return menu;
}

QMenu* Builder::createMenu( const Node& node )
{
    QMenu* menu = NULL;
    bool separator = false;

    foreach ( Node child, node.children() ) {
        if ( child.type() == Separator ) {
            if ( menu != NULL )
                separator = true;
            continue;
        }

        QAction* action = NULL;

        if ( child.type() == Action )
            action = findAction( child.id() );

        if ( child.type() == Menu ) {
            QMenu* subMenu = createMenu( child );
            if ( subMenu )
                action = subMenu->menuAction();
        }

        if ( action && action->isVisible() ) {
            if ( !menu ) {
                QString title = findTitle( node.id(), node.id() );
                menu = new QMenu( title, m_parent );
            }

            if ( separator ) {
                menu->addSeparator();
                separator = false;
            }

            menu->addAction( action );
        }
    }

    return menu;
}

QAction* Builder::findAction( const QString& id )
{
    for ( int i = m_clients.count() - 1; i >= 0; i-- ) {
        QAction* action = m_clients.at( i )->action( id );
        if ( action )
            return action;
    }

    return NULL;
}

QString Builder::findTitle( const QString& id, const QString& defaultTitle /*= QString()*/ )
{
    for ( int i = m_clients.count() - 1; i >= 0; i-- ) {
        QString title = m_clients.at( i )->title( id );
        if ( !title.isEmpty() )
            return title;
    }

    return defaultTitle;
}
