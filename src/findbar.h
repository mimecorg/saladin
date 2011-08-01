/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011 Michał Męciński
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

#ifndef FINDBAR_H
#define FINDBAR_H

#include <QWidget>
#include <QTextDocument>

class QLineEdit;
class QToolButton;
class QCheckBox;
class QLabel;

/**
* Bar for finding text.
*/
class FindBar : public QWidget
{
    Q_OBJECT
public:
    /**
    * Constructor.
    * @param parent The parent widget.
    */
    FindBar( QWidget* parent );

    /**
    * Destructor.
    */
    ~FindBar();

public:
    /**
    * Set the text to find.
    */
    void setText( const QString& text );

    /**
    * Return the text to find.
    */
    QString text() const;

    /**
    * Set the find flags.
    */
    void setFlags( QTextDocument::FindFlags flags );

    /**
    * Return the find flags.
    */
    QTextDocument::FindFlags flags() const;

    /**
    * Return @c true if find is enabled.
    */
    bool isFindEnabled() const { return m_enabled; }

    /**
    * Show or hide the "Text not found" message.
    */
    void showWarning( bool on );

    /**
    * Set the bound widget to which up/down key events are forwarded.
    */
    void setBoundWidget( QWidget* widget );

    /**
    * Select all text in the edit box.
    */
    void selectAll();

signals:
    /**
    * Emitted when the find text is updated.
    */
    void find( const QString& text );

    /**
    * Emitted when the Find Next button is clicked.
    */
    void findNext();

    /**
    * Emitted when the Find Previous button is clicked.
    */
    void findPrevious();

    /**
    * Emitted when find is enabled or disabled.
    */
    void findEnabled( bool enabled );

protected: // overrides
    void hideEvent( QHideEvent* e );

    bool eventFilter( QObject* obj, QEvent* e );

private slots:
    void textChanged( const QString& text );

    void caseToggled();

private:
    QLineEdit* m_edit;

    QToolButton* m_previousButton;
    QToolButton* m_nextButton;

    QCheckBox* m_caseCheckBox;

    QLabel* m_warningPixmap;
    QLabel* m_warningLabel;

    QWidget* m_boundWidget;

    bool m_enabled;
};

#endif
