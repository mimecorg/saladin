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

#ifndef XMLUI_TOOLSTRIP_P_H
#define XMLUI_TOOLSTRIP_P_H

#include "toolstrip.h"

#include <QLayout>
#include <QToolButton>

namespace XmlUi
{

class ToolStripLayout : public QLayout
{
    Q_OBJECT
public:
    ToolStripLayout( QWidget* parent );
    ~ToolStripLayout();

public:
    void setHeaderWidget( QWidget* widget );
    QWidget* headerWidget() const { return m_headerWidget; }

    void addLayout( ToolStripSectionLayout* layout );

    void addAuxiliaryButton( QToolButton* button );

    void clear();

    void clearAuxiliaryButtons();

public: // overrides
    Qt::Orientations expandingDirections() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    int count() const;
    QLayoutItem* itemAt( int index ) const;

    void addItem( QLayoutItem* item );
    QLayoutItem* takeAt( int index );

    void setGeometry( const QRect& rect );

    void invalidate();

private:
    void calculateSize();

    ToolStripSectionLayout* layoutAt( int index ) const;
    QToolButton* buttonAt( int index ) const;

private:
    QWidget* m_headerWidget;

    QList<QLayoutItem*> m_items;

    QList<QToolButton*> m_auxiliaryButtons;
    QToolButton* m_chevronButton;

    bool m_dirty;
    bool m_simpleLayout;
    QSize m_sizeHint;
    QSize m_minimumSize;
    QSize m_maximumSize;
    int m_auxWidth;
    int m_auxWidthNoChevron;
};

class ToolStripSectionLayout : public QLayout
{
    Q_OBJECT
public:
    ToolStripSectionLayout( const QString& title );
    ~ToolStripSectionLayout();

public:
    void addLayout( QLayout* layout );

    void drawSection( QPainter* painter, QWidget* widget );

    void setCollapsed( bool collapsed );
    bool isCollapsed() const { return m_collapsed; }

public: // overrides
    Qt::Orientations expandingDirections() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    int count() const;
    QLayoutItem* itemAt( int index ) const;

    void addItem( QLayoutItem* item );
    QLayoutItem* takeAt( int index );

    void setGeometry( const QRect& rect );

    void invalidate();

private:
    void calculateSize();

private:
    QList<QLayoutItem*> m_items;

    QString m_titleText;
    QRect m_titleRect;

    QRect m_separatorRect;

    bool m_collapsed;

    bool m_dirty;
    QSize m_sizeHint;
    QSize m_titleSize;
};

class ChevronButton : public QToolButton
{
    Q_OBJECT
public:
    ChevronButton( QWidget* parent );
    ~ChevronButton();

protected: // overrides
    void paintEvent( QPaintEvent* e );
};

class ActionButton : public QToolButton
{
    Q_OBJECT
public:
    ActionButton( QWidget* parent );
    ~ActionButton();

public:
    void adjustText();

protected: // overrides
    void actionEvent( QActionEvent* e );
};

}

#endif
