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

#ifndef XMLUI_WINDOWSSTYLE_H
#define XMLUI_WINDOWSSTYLE_H

#include <QtGlobal>

#if !defined( Q_WS_WIN ) || ( QT_VERSION < 0x040300 ) || defined( QT_NO_STYLE_WINDOWSVISTA )
#define XMLUI_NO_STYLE_WINDOWS
#endif

#if !defined( XMLUI_NO_STYLE_WINDOWS )

#include <QWindowsVistaStyle>

namespace XmlUi
{

/**
* Modern Qt style for Windows.
*
* The style imitates the look of MS Office.
*/
class WindowsStyle : public QWindowsVistaStyle
{
    Q_OBJECT
public:
    /**
    * Constructor.
    */
    WindowsStyle();

    /**
    * Destructor.
    */
    ~WindowsStyle();

public: // overrides
    void polish( QApplication* application );
    void unpolish( QApplication* application );

    void polish( QPalette& palette );

    void polish( QWidget* widget );
    void unpolish( QWidget* widget );

    int pixelMetric( PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const;

    int styleHint( StyleHint hint, const QStyleOption* option, const QWidget* widget,
        QStyleHintReturn* returnData = NULL ) const;

    QSize sizeFromContents( ContentsType type, const QStyleOption* option,
        const QSize& contentsSize, const QWidget* widget ) const;

    QRect subElementRect( SubElement element, const QStyleOption* option, const QWidget* widget ) const;
    QRect subControlRect( ComplexControl control, const QStyleOptionComplex* option,
        SubControl subControl, const QWidget* widget ) const;

    void drawPrimitive( PrimitiveElement element, const QStyleOption* option,
        QPainter* painter, const QWidget* widget ) const;
    void drawControl( ControlElement element, const QStyleOption* option,
        QPainter* painter, const QWidget* widget ) const;
    void drawComplexControl( ComplexControl control, const QStyleOptionComplex* option,
        QPainter* painter, const QWidget* widget ) const;

private:
    QColor m_colorBackgroundBegin;
    QColor m_colorBackgroundEnd;
    QColor m_colorMenuBorder;
    QColor m_colorMenuBackground;
    QColor m_colorBarBegin;
    QColor m_colorBarEnd;
    QColor m_colorSeparator;
    QColor m_colorItemBorder;
    QColor m_colorItemBackgroundBegin;
    QColor m_colorItemBackgroundMiddle;
    QColor m_colorItemBackgroundEnd;
    QColor m_colorItemCheckedBegin;
    QColor m_colorItemCheckedMiddle;
    QColor m_colorItemCheckedEnd;
    QColor m_colorItemSunkenBegin;
    QColor m_colorItemSunkenMiddle;
    QColor m_colorItemSunkenEnd;
    QColor m_colorToolStripLabel;
};

}

#endif // !defined( XMLUI_NO_STYLE_WINDOWS )

#endif
