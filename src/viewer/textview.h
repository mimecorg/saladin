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

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "viewer/view.h"

class TextEdit;
class FindBar;
class TextLoader;

class TextView : public View
{
    Q_OBJECT
public:
    TextView( QObject* parent, QWidget* parentWidget );
    ~TextView();

public: // overrides
    Type type() const;

    void load();

    void setFullScreen( bool on );

    bool eventFilter( QObject* obj, QEvent* e );

private slots:
    void updateActions();

    void copy();
    void selectAll();

    void find();
    void findNext();
    void findPrevious();

    void findText( const QString& text );

    void goToLine();

    void toggleWordWrap();
    void setEncoding( const QString& format );

    void selectEncoding();

    void contextMenuRequested( const QPoint& pos );

    void loadNextBlock();

    void settingsChanged();

private:
    void initializeSettings();
    void storeSettings();

    QMenu* createEncodingMenu();

    void findText( const QString& text, int from, QTextDocument::FindFlags flags );

private:
    TextEdit* m_edit;

    TextLoader* m_loader;

    qint64 m_length;

    QString m_encoding;

    QSignalMapper* m_encodingMapper;

    FindBar* m_findBar;

    bool m_isFindEnabled;

    int m_currentLine;
};

#endif
