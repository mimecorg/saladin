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

#ifndef XMLUI_BUILDER_H
#define XMLUI_BUILDER_H

#include "node_p.h"

#include <QObject>
#include <QList>
#include <QMap>

namespace XmlUi
{
class Node;
class Client;
class ToolStrip;
}

class QMainWindow;
class QMenu;
class QToolBar;
class QAction;

/**
* Simple XML-based UI builder for Qt4
*/
namespace XmlUi
{

/**
* The UI builder connected to a window or dialog.
*
* The builder object automatically populates the ToolStrip widgets.
* The builder can also create popup menus.
*
* The menus and toolstrips are created by merging the layout of all
* registered clients. 
*/
class Builder : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    Builder( QWidget* parent );

    /**
    * Destructor.
    */
    ~Builder();

public:
    /**
    * Add a client to the builder.
    * The UI is automatically rebuilt. The UI is merged in the same
    * order the clients are added.
    * @param client The client to add.
    */
    void addClient( Client* client );

    /**
    * Remove a client from the builder.
    * The UI is automatically rebuilt.
    * @param client The client to remove.
    */
    void removeClient( Client* client );

    /**
    * Create a context menu.
    * Note that the menu is deleted when the UI is rebuilt.
    * @param id The identifier of the menu.
    * @return The created menu or @c NULL if it wasn't found.
    */
    QMenu* contextMenu( const QString& id );

    /**
    * Register a toolstrip.
    * @param id The identifier of the toolstrip.
    * @param strip The ToolStrip to populate with actions.
    */
    void registerToolStrip( const QString& id, ToolStrip* strip );

    /**
    * Return the toolstrip with given identifier.
    */
    ToolStrip* toolStrip( const QString& id ) const;

    /**
    * Supress rebuilding the UI.
    */
    void supressUpdate();

    /**
    * Resume rebuilding the UI.
    */
    void resumeUpdate();

    /**
    * Rebuild the entire UI.
    */
    void rebuildAll();

signals:
    /**
    * Emitted when the UI is reset.
    */
    void reset();

private:
    Node mergeNodes( const Node& node1, const Node& node2 );
    bool canMergeNodes( const Node& node1, const Node& node2 );

    Node resolveGroups( const Node& node );

    void populateToolStrip( const Node& node );

    void addToolAction( ToolStrip* strip, QAction* action, const QString& id );

    QMenu* createMenu( const Node& node );

    QAction* findAction( const QString& id );
    QString findTitle( const QString& id, const QString& defaultTitle = QString() );

private:
    QWidget* m_parent;

    QList<Client*> m_clients;

    Node m_rootNode;

    QMap<QString, QMenu*> m_contextMenus;
    QMap<QString, ToolStrip*> m_toolStrips;

    bool m_updateEnabled;
    bool m_updatePending;
};

}

#endif
