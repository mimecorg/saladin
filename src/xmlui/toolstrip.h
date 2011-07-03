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

#ifndef XMLUI_TOOLSTRIP_H
#define XMLUI_TOOLSTRIP_H

#include <QWidget>

namespace XmlUi
{
class ToolStripLayout;
class ToolStripSectionLayout;
}

class QBoxLayout;
class QGridLayout;
class QToolButton;

namespace XmlUi
{

/**
* Widget containing tool buttons.
*
* This widget can be used as a replacement of the menu bar and toolbars.
* It displays tool buttons of different size layed out and grouped in sections.
* The tool strip can be populated with actions using the Builder object.
*/
class ToolStrip : public QWidget
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    ToolStrip( QWidget* parent );

    /**
    * Destructor.
    */
    ~ToolStrip();

public:
    /**
    * Set the header widget displayed in the left part of the tool strip.
    */
    void setHeaderWidget( QWidget* widget );

    /**
    * Return the header widget.
    */
    QWidget* headerWidget() const;

    /**
    * Add a tool button associated with given action.
    * @param action The action to add.
    */
    void addToolAction( QAction* action );

    /**
    * Add a separator between buttons.
    */
    void addSeparator();

    /**
    * Begin a section containing actions.
    * @param title The title of the section.
    */
    void beginSection( const QString& title );

    /**
    * End the current section.
    */
    void endSection();

    /**
    * Begin a grid layout inside a section.
    */
    void beginGrid();

    /**
    * End the current grid layout.
    */
    void endGrid();

    /**
    * Begin a horizontal row of small buttons in a grid layout.
    */
    void beginRow();

    /**
    * End the current row.
    */
    void endRow();

    /**
    * Remove all tool buttons and sections.
    * The header widget and auxiliary actions are not removed.
    */
    void clearToolActions();

    /**
    * Add a tool button associated with given action in the top right corner
    * of the tool strip.
    * @param action The action to add.
    */
    void addAuxiliaryAction( QAction* action );

    /**
    * Remove all auxiliary actions.
    */
    void clearAuxiliaryActions();

    /**
    * Set the margins for tool strip contents.
    */
    void setContentsMargins( int left, int top, int right, int bottom );

protected: // overrides
    void childEvent( QChildEvent* e );
    void paintEvent( QPaintEvent* e );

private:
    enum ButtonSize {
        SmallButton,
        MediumButton,
        LargeButton
    };

private:
    QToolButton* createButton( QAction* action, ButtonSize size );

private:
    ToolStripLayout* m_layout;

    ToolStripSectionLayout* m_sectionLayout;

    QGridLayout* m_gridLayout;
    int m_gridRow;
    int m_gridColumn;

    QBoxLayout* m_rowLayout;

    QList<QToolButton*> m_toolButtons;
};

/**
* Action with extended properties.
*/
class ToolStripAction : public QAction
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param text The text of the action.
    * @param parent The parent object.
    */
    ToolStripAction( const QString& text, QObject* parent );

    /**
    * Constructor.
    * @param icon The icon of the action.
    * @param text The text of the action.
    * @param parent The parent object.
    */
    ToolStripAction( const QIcon& icon, const QString& text, QObject* parent );

    /**
    * Destructor.
    */
    ~ToolStripAction();

public:
    /**
    * Set the menu popup mode for the tool button.
    */
    void setPopupMode( QToolButton::ToolButtonPopupMode mode );

    /**
    * Return the menu popup mode for the tool button.
    */
    QToolButton::ToolButtonPopupMode popupMode() const { return m_popupMode; }

private:
    QToolButton::ToolButtonPopupMode m_popupMode;
};

}

#endif
