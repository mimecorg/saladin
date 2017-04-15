/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011-2017 Michał Męciński
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

#include "textedit.h"

#include "application.h"
#include "utils/localsettings.h"

class LineNumberArea : public QWidget
{
public:
    LineNumberArea( TextEdit* parent ) : QWidget( parent ),
        m_textEdit( parent )
    {
    }

public:
    QSize sizeHint() const
    {
        return QSize( m_textEdit->lineNumberAreaWidth(), 0 );
    }

protected:
    void paintEvent( QPaintEvent* e )
    {
        m_textEdit->lineNumberAreaPaintEvent( e );
    }

private:
    TextEdit* m_textEdit;
};

TextEdit::TextEdit( QWidget* parent ) : QPlainTextEdit( parent ),
    m_lineNumbers( false )
{
    m_lineNumberArea = new LineNumberArea( this );
    m_lineNumberArea->hide();

    setBackgroundRole( QPalette::Base );
    setAutoFillBackground( true );

    connect( this, SIGNAL( blockCountChanged( int ) ), this, SLOT( updateLineNumberAreaWidth() ) );
    connect( this, SIGNAL( updateRequest( const QRect&, int ) ), this, SLOT( updateLineNumberArea( const QRect&, int ) ) );

    updateLineNumberAreaWidth();

    connect( application, SIGNAL( themeChanged() ), this, SLOT( themeChanged() ) );
}

TextEdit::~TextEdit()
{
}

void TextEdit::setLineNumbers( bool numbers )
{
    if ( m_lineNumbers != numbers ) {
        m_lineNumbers = numbers;
        m_lineNumberArea->setVisible( numbers );
        updateLineNumberAreaWidth();
    }
}

int TextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax( 1, blockCount() );
    while ( max >= 10 ) {
        max /= 10;
        digits++;
    }

    return fontMetrics().width( QLatin1Char( '9' ) ) * digits + 4;
}

void TextEdit::updateLineNumberAreaWidth()
{
    setViewportMargins( ( m_lineNumbers ? lineNumberAreaWidth() : 0 ) + 2, 0, 0, 0 );
}

void TextEdit::updateLineNumberArea( const QRect& rect, int dy )
{
    if ( m_lineNumbers ) {
        if ( dy )
            m_lineNumberArea->scroll( 0, dy );
        else
            m_lineNumberArea->update( 0, rect.y(), m_lineNumberArea->width(), rect.height() );

        if ( rect.contains( viewport()->rect() ) )
            updateLineNumberAreaWidth();
    }
}

void TextEdit::resizeEvent( QResizeEvent* e )
{
    QPlainTextEdit::resizeEvent( e );

    QRect rect = contentsRect();
    m_lineNumberArea->setGeometry( QRect( rect.left(), rect.top(), lineNumberAreaWidth(), rect.height() ) );
}

void TextEdit::lineNumberAreaPaintEvent( QPaintEvent* e )
{
    QString theme = application->applicationSettings()->value( "Theme" ).toString();

    QColor background;
    QColor foreground;

    if ( theme == QLatin1String( "dark" ) ) {
        background = QColor( 16, 16, 16 );
        foreground = QColor( 128, 128, 128 );
    } else {
        background = QColor( 228, 228, 228 );
        foreground = QColor( 128, 128, 128 );
    }

    QPainter painter( m_lineNumberArea );
    painter.fillRect( e->rect(), background );

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry( block ).translated( contentOffset() ).top();
    int bottom = top + (int)blockBoundingRect( block ).height();

    painter.setPen( foreground );

    while ( block.isValid() && top <= e->rect().bottom() ) {
        if ( block.isVisible() && bottom >= e->rect().top() ) {
            QString number = QString::number( blockNumber + 1 );
            painter.drawText( 0, top, m_lineNumberArea->width() - 2, fontMetrics().height(), Qt::AlignRight, number );
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect( block ).height();
        blockNumber++;
    }
}

void TextEdit::themeChanged()
{
    if ( m_lineNumbers )
        m_lineNumberArea->update();
}
