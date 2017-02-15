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

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "viewer/view.h"

class ImageLabel;
class ImageLoader;

class ImageView : public View
{
    Q_OBJECT
public:
    ImageView( QObject* parent, QWidget* parentWidget );
    ~ImageView();

public: // overrides
    Type type() const;

    void load();

    void storeSettings();

private slots:
    void updateActions();

    void copy();

    void zoomFit();

    void zoomIn();
    void zoomOut();
    void zoomOriginal();

    void rotateLeft();
    void rotateRight();

    void blackBackground();

    void contextMenuRequested( const QPoint& pos );

    void loadImage();

    void updateStatus();

private:
    void initializeSettings();

    void zoom( double factor );
    void adjustScrollBar( QScrollBar* scrollBar, double factor );

private:
    QScrollArea* m_scroll;

    ImageLabel* m_label;

    ImageLoader* m_loader;
};

#endif
