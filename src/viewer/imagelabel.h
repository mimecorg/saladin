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

#ifndef IMAGELABEL_H
#define IMAGELABEL_H

class ImageLabel : public QWidget
{
    Q_OBJECT
public:
    ImageLabel( QWidget* parent );
    ~ImageLabel();

public:
    void setImage( const QImage& image );
    const QImage& image() const { return m_image; }

    void setZoom( double zoom );
    double zoom() const { return m_zoom; }

public: // overrides
    QSize sizeHint() const;

protected: // overrides
    void paintEvent( QPaintEvent* e );

    void wheelEvent( QWheelEvent* e );

signals:
    void zoomIn();
    void zoomOut();

private:
    QImage m_image;

    double m_zoom;
};

#endif
