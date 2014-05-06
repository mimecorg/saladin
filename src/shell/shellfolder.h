/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2014 Michał Męciński
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

#ifndef SHELLFOLDER_H
#define SHELLFOLDER_H

#include "shellitem.h"
#include "shellselection.h"

#include <QObject>

class ShellFolderPrivate;
class ShellPidl;
class ItemChange;

class ShellFolder : public QObject
{
    Q_OBJECT
public:
    enum Flag
    {
        IncludeHidden = 1
    };
    Q_DECLARE_FLAGS( Flags, Flag );

public:
    ShellFolder( const QString& path, QWidget* parent );
    ShellFolder( const ShellPidl& pidl, QWidget* parent );
    ~ShellFolder();

public:
    bool isValid() const;
    QString path() const;
    bool hasParent() const;

    ShellItem::Attributes attributes();
    QString name();

    ShellPidl pidl() const;

    QString user() const;
    QString password() const;

    QList<ShellItem> listItems( Flags flags );

    bool extractIcon( ShellItem& item );

    bool calculateSize( ShellItem& item );

    bool setItemName( ShellItem& item, const QString& name );

    QString itemPath( const ShellItem& item ) const;
    ShellPidl itemPidl( const ShellItem& item ) const;

    bool isEqual( const ShellFolder* other ) const;

    ShellFolder* clone();

    ShellFolder* openFolder( const ShellItem& item );
    ShellFolder* parentFolder( ShellItem& item );
    ShellFolder* rootFolder();

    ShellFolder* browseFolder();

    bool canCreateFolder();
    bool createFolder( const QString& name );

    bool createFile( const QString& name, bool overwrite, const char* data = NULL, int size = 0 );

    ShellItem childItem( const QString& name );

    bool executeItem( const ShellItem& item );

    ShellSelection::MenuCommand showContextMenu( const QPoint& pos, ShellSelection::Flags flags );

    bool invokeCommand( const char* verb );

    bool search();
    bool explore();

    QString toolTip( const ShellItem& item );

    bool startWatching();

public:
    static ShellPidl defaultFolder();

    static ShellPidl browseFolder( QWidget* parent, const QString& title, const ShellPidl& startPidl );

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>( QObject::parent() ); }

signals:
    void itemChanged( const ItemChange& change );
    void folderUpdated();

private slots:
    void changeNotify( int eventType, void* arg1, void* arg2 );

private:
    ShellFolder( IShellFolder* folder, QWidget* parent );

private:
    ShellFolderPrivate* d;

    friend class ShellFolderPrivate;
    friend class ShellComputer;
    friend class ShellSelection;
    friend class ShellSelectionPrivate;
    friend class ShellDropData;
};

class ItemChange
{
public:
    enum Type
    {
        ItemAdded,
        ItemRemoved,
        ItemUpdated,
        ItemRenamed
    };

public:
    ItemChange( Type type, const ShellItem& item1, const ShellItem& item2 = ShellItem() );
    ~ItemChange();

public:
    ItemChange( const ItemChange& other );
    ItemChange& operator =( const ItemChange& other );

public:
    Type type() const { return m_type; }

    const ShellItem& item1() const { return m_item1; }
    ShellItem& item1() { return m_item1; }

    const ShellItem& item2() const { return m_item2; }
    ShellItem& item2() { return m_item2; }

private:
    Type m_type;
    ShellItem m_item1;
    ShellItem m_item2;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( ShellFolder::Flags );

#endif
