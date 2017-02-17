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

#include "imageview.h"
#include "imagelabel.h"
#include "imageloader.h"

#include "application.h"
#include "shell/streamdevice.h"
#include "utils/localsettings.h"
#include "utils/iconloader.h"
#include "xmlui/builder.h"

ImageView::ImageView( QObject* parent, QWidget* parentWidget ) : View( parent ),
    m_loader( NULL )
{
    QAction* action;

    action = new QAction( IconLoader::icon( "edit-copy" ), tr( "&Copy" ), this );
    action->setShortcut( QKeySequence::Copy );
    connect( action, SIGNAL( triggered() ), this, SLOT( copy() ) );
    setAction( "copy", action );

    action = new QAction( IconLoader::icon( "zoom-fit" ), tr( "Zoom To &Fit" ), this );
    action->setShortcut( Qt::Key_F );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( zoomFit() ) );
    setAction( "zoomFit", action );

    action = new QAction( IconLoader::icon( "zoom-in" ), tr( "Zoom &In" ), this );
    action->setShortcut( Qt::CTRL + Qt::Key_Equal );
    connect( action, SIGNAL( triggered() ), this, SLOT( zoomIn() ) );
    setAction( "zoomIn", action );

    action = new QAction( IconLoader::icon( "zoom-out" ), tr( "Zoom &Out" ), this );
    action->setShortcut( Qt::CTRL + Qt::Key_Minus );
    connect( action, SIGNAL( triggered() ), this, SLOT( zoomOut() ) );
    setAction( "zoomOut", action );

    action = new QAction( IconLoader::icon( "zoom-orig" ), tr( "Original &Size" ), this );
    action->setShortcut( Qt::CTRL + Qt::Key_0 );
    connect( action, SIGNAL( triggered() ), this, SLOT( zoomOriginal() ) );
    setAction( "zoomOriginal", action );

    action = new QAction( IconLoader::icon( "rotate-left" ), tr( "Rotate &Left" ), this );
    action->setShortcut( Qt::Key_L );
    connect( action, SIGNAL( triggered() ), this, SLOT( rotateLeft() ) );
    setAction( "rotateLeft", action );

    action = new QAction( IconLoader::icon( "rotate-right" ), tr( "Rotate &Right" ), this );
    action->setShortcut( Qt::Key_R );
    connect( action, SIGNAL( triggered() ), this, SLOT( rotateRight() ) );
    setAction( "rotateRight", action );

    action = new QAction( IconLoader::icon( "black-background" ), tr( "&Black Background" ), this );
    action->setShortcut( Qt::Key_B );
    action->setCheckable( true );
    connect( action, SIGNAL( triggered() ), this, SLOT( blackBackground() ) );
    setAction( "blackBackground", action );

    action = new QAction( IconLoader::icon( "info" ), tr( "Information" ), this );
    action->setShortcut( Qt::Key_I );
    connect( action, SIGNAL( triggered() ), this, SLOT( viewInformation() ) );
    setAction( "viewInformation", action );

    loadXmlUiFile( ":/resources/imageview.xml" );

    QWidget* main = new QWidget( parentWidget );
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setContentsMargins( 3, 0, 3, 0 );
    mainLayout->setSpacing( 0 );

    m_scroll = new QScrollArea( main );
    m_scroll->setWidgetResizable( true );

    QPalette scrollPalette = parentWidget->palette();
    scrollPalette.setColor( QPalette::Window, QColor::fromRgb( 255, 255, 255 ) );
    m_scroll->setPalette( scrollPalette );

    m_label = new ImageLabel( m_scroll );
    m_label->setContextMenuPolicy( Qt::CustomContextMenu );

    m_scroll->setWidget( m_label );

    connect( m_label, SIGNAL( zoomIn() ), this, SLOT( zoomIn() ) );
    connect( m_label, SIGNAL( zoomOut() ), this, SLOT( zoomOut() ) );

    connect( m_label, SIGNAL( zoomChanged() ), this, SLOT( updateStatus() ) );
    
    connect( m_label, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( contextMenuRequested( const QPoint& ) ) );

    mainLayout->addWidget( m_scroll );

    main->setFocusProxy( m_scroll );

    setMainWidget( main );

    setStatus( tr( "Image" ) );

    initializeSettings();

    updateActions();
}

ImageView::~ImageView()
{
    if ( m_loader ) {
        m_loader->abort();
        m_loader = NULL;
    }

    storeSettings();
}

View::Type ImageView::type() const
{
    return Image;
}

void ImageView::initializeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    bool fit = settings->value( "ZoomToFit", false ).toBool();
    action( "zoomFit" )->setChecked( fit );
    m_label->setZoom( fit ? -1.0 : 1.0 );

    bool black = settings->value( "BlackBackground", false ).toBool();
    action( "blackBackground" )->setChecked( black );
    m_label->setBlackBackground( black );
}

void ImageView::storeSettings()
{
    LocalSettings* settings = application->applicationSettings();

    settings->setValue( "ZoomToFit", m_label->zoom() < 0.0 );
    settings->setValue( "BlackBackground", m_label->isBlackBackground() );
}

void ImageView::load()
{
    if ( m_loader ) {
        m_loader->abort();
        m_loader = NULL;
    }

    m_label->setImage( QImage() );

    m_loader = new ImageLoader( pidl() );

    connect( m_loader, SIGNAL( imageAvailable() ), this, SLOT( loadImage() ), Qt::QueuedConnection );

    m_loader->start();

    setStatus( tr( "Image" ) );
}

void ImageView::loadImage()
{
    QImage image = m_loader->image();

    QByteArray format = m_loader->format();

    m_label->setImage( image );

    updateActions();
    updateStatus();
}

void ImageView::updateActions()
{
    bool hasImage = !m_label->image().isNull();
    double zoom = m_label->zoom();

    action( "copy" )->setEnabled( hasImage );
    action( "zoomIn" )->setEnabled( hasImage && zoom > 0.0 && zoom < 20.0 );
    action( "zoomOut" )->setEnabled( hasImage && zoom > 0.05 );
    action( "zoomOriginal" )->setEnabled( hasImage && zoom > 0.0 && !qFuzzyIsNull( zoom - 1.0 ) );
    action( "rotateLeft" )->setEnabled( hasImage );
    action( "rotateRight" )->setEnabled( hasImage );
}

void ImageView::updateStatus()
{
    if ( !m_label->image().isNull() )
        setStatus( tr( "Image" ) + ", " + format().toUpper() + QString( " (%1 x %2, %3%)" ).arg( m_label->image().width() ).arg( m_label->image().height() ).arg( (int)( m_label->actualZoom() * 100.0 ) ) );
}

void ImageView::copy()
{
    QImage image = m_label->image();
    if ( !image.isNull() )
        QApplication::clipboard()->setImage( image );
}

void ImageView::zoomFit()
{
    bool checked = action( "zoomFit" )->isChecked();

    m_label->setZoom( checked ? -1.0 : 1.0 );

    updateActions();
}

void ImageView::zoomIn()
{
    if ( m_label->zoom() > 0.0 && m_label->zoom () < 20.0 )
        zoom( 1.25 );
}

void ImageView::zoomOut()
{
    if ( m_label->zoom() > 0.05 )
        zoom( 0.8 );
}

void ImageView::zoomOriginal()
{
    zoom( 1.0 / m_label->zoom() );
}

void ImageView::zoom( double factor )
{
    m_label->setZoom( factor * m_label->zoom() );

    adjustScrollBar( m_scroll->horizontalScrollBar(), factor );
    adjustScrollBar( m_scroll->verticalScrollBar(), factor );

    updateActions();
}

void ImageView::adjustScrollBar( QScrollBar* scrollBar, double factor )
{
    scrollBar->setValue( int( factor * scrollBar->value() + ( ( factor - 1.0 ) * scrollBar->pageStep() / 2.0 ) ) );
}

void ImageView::rotateLeft()
{
    QTransform transform;
    transform.rotate( -90 );

    m_label->setImage( m_label->image().transformed( transform ) );

    updateStatus();
}

void ImageView::rotateRight()
{
    QTransform transform;
    transform.rotate( 90 );

    m_label->setImage( m_label->image().transformed( transform ) );

    updateStatus();
}

void ImageView::blackBackground()
{
    bool black = action( "blackBackground" )->isChecked();

    m_label->setBlackBackground( black );
}

void ImageView::viewInformation()
{
    const QImage& image = m_label->image();

    if ( !image.isNull() ) {
        ShellPidl filePidl = pidl();

        QString path = filePidl.path();

        StreamDevice device( filePidl );

        QString info = tr( "File path: %1\nFile size: %2 bytes\nLast modified: %3" ).arg(
            path,
            QLocale::system().toString( device.size() ),
            device.lastModified().toString( "yyyy-MM-dd hh:mm" ) );

        info += "\n\n";

        info += tr( "Image format: %1" ).arg( QString( format().toUpper() ) );

        info += "\n";

        info += tr( "Image size: %1 x %2 pixels" )
            .arg( image.width() )
            .arg( image.height() );

        if ( format() == "png" ) {
            double dpiX = image.dotsPerMeterX() * 0.0254;
            double dpiY = image.dotsPerMeterY() * 0.0254;

            double cmX = image.width() / ( image.dotsPerMeterX() * 0.01 );
            double cmY = image.height() / ( image.dotsPerMeterY() * 0.01 );

            double inchX = image.width() / dpiX;
            double inchY = image.height() / dpiX;

            info += "\n";

            info += tr( "Print size: %1 x %2 cm / %3 x %4 \" (%5 x %6 dpi)" )
                .arg( cmX, 0, 'f', 2 )
                .arg( cmY, 0, 'f', 2 )
                .arg( inchX, 0, 'f', 2 )
                .arg( inchY, 0, 'f', 2 )
                .arg( dpiX, 0, 'f', 0 )
                .arg( dpiY, 0, 'f', 0 );
        }

        int colorCount = image.colorCount();
        if ( colorCount == 0 )
            colorCount = 1 << qMin( image.bitPlaneCount(), 24 );

        info += "\n";

        info += tr( "Colors: %1 (%2 bpp)\nAlpha channel: %3" )
            .arg( colorCount )
            .arg( image.bitPlaneCount() )
            .arg( image.hasAlphaChannel() ? tr( "yes" ) : tr( "no" ) );

        QMessageBox::information( mainWidget(), tr( "Information" ), info, QMessageBox::Ok );
    }
}

void ImageView::contextMenuRequested( const QPoint& pos )
{
    QMenu* menu = builder()->contextMenu( "menuContext" );
    if ( menu )
        menu->popup( m_label->mapToGlobal( pos ) );
}
