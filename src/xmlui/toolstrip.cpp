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

#include "toolstrip.h"
#include "toolstrip_p.h"

#include <QLayout>
#include <QAction>
#include <QToolButton>
#include <QEvent>
#include <QMenu>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QActionEvent>
#include <qdrawutil.h>

using namespace XmlUi;

ToolStrip::ToolStrip( QWidget* parent ) : QWidget( parent ),
    m_sectionLayout( NULL ),
    m_gridLayout( NULL ),
    m_gridRow( 0 ),
    m_gridColumn( 0 ),
    m_rowLayout( NULL )
{
    m_layout = new ToolStripLayout( this );
}

ToolStrip::~ToolStrip()
{
}

void ToolStrip::setHeaderWidget( QWidget* widget )
{
    delete m_layout->headerWidget();

    if ( widget )
        m_layout->setHeaderWidget( widget );
}

QWidget* ToolStrip::headerWidget() const
{
    return m_layout->headerWidget();
}

void ToolStrip::addToolAction( QAction* action )
{
    QToolButton* button;

    if ( m_rowLayout ) {
        button = createButton( action, SmallButton );
        m_rowLayout->addWidget( button );
    } else if ( m_gridLayout ) {
        button = createButton( action, MediumButton );
        m_gridLayout->addWidget( button, m_gridRow++, m_gridColumn );
        if ( m_gridRow == 2 ) {
            m_gridRow = 0;
            m_gridColumn++;
        }
    } else if ( m_sectionLayout ) {
        button = createButton( action, LargeButton );
        m_sectionLayout->addWidget( button );
    } else {
        button = createButton( action, MediumButton );
        m_layout->addWidget( button );
    }

    m_toolButtons.append( button );
}

void ToolStrip::addSeparator()
{
    m_layout->addItem( new QSpacerItem( 7, 0 ) );
}

void ToolStrip::beginSection( const QString& title )
{
    m_sectionLayout = new ToolStripSectionLayout( title );
    m_layout->addLayout( m_sectionLayout );
}

void ToolStrip::endSection()
{
    m_sectionLayout = NULL;
}

void ToolStrip::beginGrid()
{
    m_gridLayout = new QGridLayout();
    m_gridLayout->setMargin( 0 );
    m_gridLayout->setSpacing( 0 );

    m_sectionLayout->addLayout( m_gridLayout );

    m_gridColumn = 0;
    m_gridRow = 0;
}

void ToolStrip::endGrid()
{
    m_gridLayout = NULL;
}

void ToolStrip::beginRow()
{
    m_rowLayout = new QHBoxLayout();
    m_rowLayout->setMargin( 0 );
    m_rowLayout->setSpacing( 0 );

    m_gridLayout->addLayout( m_rowLayout, m_gridRow++, m_gridColumn );
    if ( m_gridRow == 2 ) {
        m_gridRow = 0;
        m_gridColumn++;
    }
}

void ToolStrip::endRow()
{
    m_rowLayout->addStretch( 1 );
    m_rowLayout = NULL;
}

void ToolStrip::clearToolActions()
{
    while ( !m_toolButtons.isEmpty() ) {
        QToolButton* button = m_toolButtons.takeFirst();
        button->hide();
        button->deleteLater();
    }

    m_layout->clear();
}

void ToolStrip::addAuxiliaryAction( QAction* action )
{
    QToolButton* button = createButton( action, SmallButton );
    m_layout->addAuxiliaryButton( button );
}

void ToolStrip::clearAuxiliaryActions()
{
    m_layout->clearAuxiliaryButtons();
}

void ToolStrip::setContentsMargins( int left, int top, int right, int bottom )
{
    m_layout->setContentsMargins( left, top, right, bottom );
}

void ToolStrip::execMenu( QAction* action )
{
    foreach ( QToolButton* button, m_toolButtons ) {
        if ( button->defaultAction() == action ) {
            button->setDown( true );
            action->menu()->exec( button->mapToGlobal( button->rect().bottomLeft() + QPoint( 0, 1 ) ) );
            button->setDown( false );
            break;
        }
    }
}

QToolButton* ToolStrip::createButton( QAction* action, ButtonSize size )
{
    ActionButton* button = new ActionButton( this );

    switch ( size ) {
        case SmallButton:
            button->setToolButtonStyle( Qt::ToolButtonIconOnly );
            button->setIconSize( QSize( 16, 16 ) );
            break;
        case MediumButton:
            button->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
            button->setIconSize( QSize( 16, 16 ) );
            break;
        case LargeButton:
            button->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
            button->setIconSize( QSize( 22, 22 ) );
            break;
    }

    button->setDefaultAction( action );
    button->adjustText();

    if ( ToolStripAction* tsAction = qobject_cast<ToolStripAction*>( action ) )
        button->setPopupMode( tsAction->popupMode() );

    return button;
}

void ToolStrip::childEvent( QChildEvent* e )
{
    QWidget::childEvent( e );

    if ( e->type() == QEvent::ChildRemoved && e->child() == m_layout->headerWidget() )
        m_layout->setHeaderWidget( NULL );
}

static void drawSeparator( const QRect& rect, QPainter* painter, QWidget* widget )
{
#if ( QT_VERSION >= 0x040500 )
    QStyleOptionFrameV3 option;
    option.init( widget );
    option.frameShape = QFrame::VLine;
    option.state |= QStyle::State_Sunken;
    option.rect = rect;
    option.lineWidth = 1;
    widget->style()->drawControl( QStyle::CE_ShapedFrame, &option, painter, widget );
#else
    int x = ( rect.left() + rect.right() ) / 2;
    qDrawShadeLine( painter, QPoint( x, rect.top() ), QPoint( x, rect.bottom() ), widget->palette(), true, 1, 0 );
#endif
}

void ToolStrip::paintEvent( QPaintEvent* /*e*/ )
{
    QPainter painter( this );

    for ( int i = 0; i < m_layout->count(); i++ ) {
        QLayout* childLayout = m_layout->itemAt( i )->layout();
        if ( childLayout ) {
            ToolStripSectionLayout* sectionLayout = qobject_cast<ToolStripSectionLayout*>( childLayout );
            if ( sectionLayout && !sectionLayout->isCollapsed() )
                sectionLayout->drawSection( &painter, this );
        }

        QSpacerItem* spacer = m_layout->itemAt( i )->spacerItem();
        if ( spacer ) {
            QRect rect = spacer->geometry();
            if ( !rect.isEmpty() )
                drawSeparator( rect, &painter, this );
        }
    }
}

ToolStripAction::ToolStripAction( const QString& text, QObject* parent ) : QAction( text, parent ),
    m_popupMode( QToolButton::MenuButtonPopup )
{
}

ToolStripAction::ToolStripAction( const QIcon& icon, const QString& text, QObject* parent ) : QAction( icon, text, parent ),
    m_popupMode( QToolButton::MenuButtonPopup )
{
}

ToolStripAction::~ToolStripAction()
{
}

void ToolStripAction::setPopupMode( QToolButton::ToolButtonPopupMode mode )
{
    m_popupMode = mode;
}

ToolStripLayout::ToolStripLayout( QWidget* parent ) : QLayout( parent ),
    m_headerWidget( NULL ),
    m_dirty( true ),
    m_simpleLayout( true ),
    m_auxWidth( 0 ),
    m_auxWidthNoChevron( 0 )
{
    setContentsMargins( 0, 0, 0, 0 );

    m_chevronButton = new ChevronButton( parent );
    addChildWidget( m_chevronButton );
}

ToolStripLayout::~ToolStripLayout()
{
    clear();
}

void ToolStripLayout::setHeaderWidget( QWidget* widget )
{
    if ( widget )
        addChildWidget( widget );
    m_headerWidget = widget;
    invalidate();
}

void ToolStripLayout::addLayout( ToolStripSectionLayout* layout )
{
    addChildLayout( layout );
    addItem( layout );
}

void ToolStripLayout::addAuxiliaryButton( QToolButton* button )
{
    addChildWidget( button );
    m_auxiliaryButtons.append( button );
    invalidate();
}

void ToolStripLayout::clear()
{
    while ( !m_items.isEmpty() )
        delete m_items.takeFirst();
    invalidate();
}

void ToolStripLayout::clearAuxiliaryButtons()
{
    while ( !m_auxiliaryButtons.isEmpty() ) {
        QToolButton* button = m_auxiliaryButtons.takeFirst();
        button->hide();
        button->deleteLater();
    }
    invalidate();
}

Qt::Orientations ToolStripLayout::expandingDirections() const
{
    return Qt::Horizontal;
}

QSize ToolStripLayout::sizeHint() const
{
    if ( m_dirty )
        const_cast<ToolStripLayout*>( this )->calculateSize();
    return m_sizeHint;
}

QSize ToolStripLayout::minimumSize() const
{
    if ( m_dirty )
        const_cast<ToolStripLayout*>( this )->calculateSize();
    return m_minimumSize;
}

QSize ToolStripLayout::maximumSize() const
{
    if ( m_dirty )
        const_cast<ToolStripLayout*>( this )->calculateSize();
    return m_maximumSize;
}

void ToolStripLayout::calculateSize()
{
    m_simpleLayout = true;

    int width = 0;
    int height = 0;

    if ( m_headerWidget ) {
        QSize size = m_headerWidget->sizeHint();
        width = size.width();
        height = size.height();
        m_simpleLayout = false;
    }

    for ( int i = 0; i < m_items.count(); i++ ) {
        QSize size;
        if ( ToolStripSectionLayout* layout = layoutAt( i ) ) {
            size = layout->sizeHint();
            m_simpleLayout = false;
        } else if ( QToolButton* button = buttonAt( i ) ) {
            size = button->sizeHint();
        } else if ( QSpacerItem* spacer = itemAt( i )->spacerItem() ) {
            size = spacer->sizeHint();
        }
        width += size.width();
        height = qMax( height, size.height() );
    }

    int auxWidth = 0;
    int auxHeight = 0;

    QSize chevronSize = m_chevronButton->sizeHint();

    foreach ( QToolButton* button, m_auxiliaryButtons ) {
        QSize size = button->sizeHint();
        auxWidth += size.width();
        auxHeight = qMax( auxHeight, size.height() );
    }

    if ( m_simpleLayout ) {
        m_auxWidthNoChevron = auxWidth > 0 ? auxWidth + 3 : 0;
        m_auxWidth = m_auxWidthNoChevron + chevronSize.width() + 3;
    } else {
        m_auxWidthNoChevron = auxWidth > 0 ? auxWidth + 6 : 0;
        m_auxWidth = qMax( m_auxWidthNoChevron, chevronSize.width() + 6 );
        auxHeight += chevronSize.height() + 9;
    }

    int ml, mt, mr, mb;
    getContentsMargins( &ml, &mt, &mr, &mb );

    m_sizeHint = QSize( width + m_auxWidthNoChevron + ml + mr, qMax( height, auxHeight ) + mt + mb );
    m_minimumSize = QSize( m_auxWidth + ml + mr, qMax( height, auxHeight ) + mt + mb );
    m_maximumSize = QSize( QLAYOUTSIZE_MAX, qMax( height, auxHeight ) + mt + mb );

    m_dirty = false;
}

int ToolStripLayout::count() const
{
    return m_items.count();
}

QLayoutItem* ToolStripLayout::itemAt( int index ) const
{
    return ( index >= 0 && index < m_items.count() ) ? m_items.at( index ) : NULL;
}

void ToolStripLayout::addItem( QLayoutItem* item )
{
    m_items.append( item );
    invalidate();
}

QLayoutItem* ToolStripLayout::takeAt( int index )
{
    QLayoutItem* item = m_items.takeAt( index );
    invalidate();
    return item;
}

ToolStripSectionLayout* ToolStripLayout::layoutAt( int index ) const
{
    QLayoutItem* item = itemAt( index );
    if ( item && item->layout() )
        return qobject_cast<ToolStripSectionLayout*>( item->layout() );
    return NULL;
}

QToolButton* ToolStripLayout::buttonAt( int index ) const
{
    QLayoutItem* item = itemAt( index );
    if ( item && item->widget() )
        return qobject_cast<QToolButton*>( item->widget() );
    return NULL;
}

static void addActions( QLayoutItem* item, QMenu* menu )
{
    QLayout* layout = item->layout();
    if ( layout ) {
        for ( int i = 0; i < layout->count(); i++ )
            addActions( layout->itemAt( i ), menu );
    }

    QWidget* widget = item->widget();
    if ( widget ) {
        QToolButton* button = qobject_cast<QToolButton*>( widget );
        if ( button && button->defaultAction() )
            menu->addAction( button->defaultAction() );
    }
}

void ToolStripLayout::setGeometry( const QRect& rect )
{
    if ( !m_dirty && geometry() == rect )
        return;

    calculateSize();

    QLayout::setGeometry( rect );

    QRect contents = contentsRect();

    int left = contents.left();

    bool collapsed = false;

    if ( m_headerWidget ) {
        QSize size = m_headerWidget->sizeHint();
        if ( left + size.width() > contents.right() - ( m_items.isEmpty() ? m_auxWidthNoChevron : m_auxWidth ) )
            collapsed = true;

        m_headerWidget->setVisible( !collapsed );
        if ( !collapsed ) {
            m_headerWidget->setGeometry( QRect( contents.left(), contents.top(), size.width(), contents.height() ) );
            left += size.width();
        }
    }

    m_chevronButton->menu()->clear();

    for ( int i = 0; i < m_items.count(); i++ ) {
        QSize size;
        if ( ToolStripSectionLayout* layout = layoutAt( i ) )
            size = layout->sizeHint();
        else if ( QToolButton* button = buttonAt( i ) )
            size = button->sizeHint();
        else if ( QSpacerItem* spacer = itemAt( i )->spacerItem() )
            size = spacer->sizeHint();

        if ( !collapsed && left + size.width() > contents.right() - m_auxWidth + 1 ) {
            if ( i < m_items.count() - 1 || left + size.width() > contents.right() - m_auxWidthNoChevron + 1 ) {
                if ( i > 0 ) {
                    if ( QSpacerItem* lastSpacer = itemAt( i - 1 )->spacerItem() ) {
                        left -= 7;
                        lastSpacer->setGeometry( QRect( left, contents.top(), 0, contents.height() ) );
                    }
                }
                collapsed = true;
            }
        }

        if ( ToolStripSectionLayout* layout = layoutAt( i ) ) {
            layout->setCollapsed( collapsed );
            if ( collapsed ) {
                if ( !m_chevronButton->menu()->isEmpty() )
                    m_chevronButton->menu()->addSeparator();
                addActions( layout, m_chevronButton->menu() );
            }
        } else if ( QToolButton* button = buttonAt( i ) ) {
            button->setVisible( !collapsed );
            if ( collapsed && button->defaultAction() )
                m_chevronButton->menu()->addAction( button->defaultAction() );
        } else if ( QSpacerItem* spacer = itemAt( i )->spacerItem() ) {
            if ( collapsed ) {
                spacer->setGeometry( QRect( left, contents.top(), 0, contents.height() ) );
                if ( !m_chevronButton->menu()->isEmpty() )
                    m_chevronButton->menu()->addSeparator();
            }
        }

        if ( !collapsed ) {
            itemAt( i )->setGeometry( QRect( left, contents.top(), size.width(), contents.height() ) );
            left += size.width();
        }
    }

    int right = contents.right() + 1;

    for ( int i = m_auxiliaryButtons.count() - 1; i >= 0; i-- ) {
        QToolButton* button = m_auxiliaryButtons.at( i );
        QSize size = button->sizeHint();
        if ( m_simpleLayout )
            button->setGeometry( QRect( right - size.width(), contents.top(), size.width(), size.height() ) );
        else
            button->setGeometry( QRect( right - size.width() - 3, contents.top() + 3, size.width(), size.height() ) );
        right -= size.width();
    }

    if ( !collapsed || m_chevronButton->menu()->isEmpty() ) {
        m_chevronButton->hide();
    } else {
        m_chevronButton->show();
        QSize size = m_chevronButton->sizeHint();
        if ( m_simpleLayout )
            m_chevronButton->setGeometry( QRect( left + 3, contents.top(), size.width(), size.height() ) );
        else
            m_chevronButton->setGeometry( QRect( left + 3, contents.bottom() - size.height() - 2, size.width(), size.height() ) );
    }
}

void ToolStripLayout::invalidate()
{
    m_dirty = true;
    QLayout::invalidate();

    if ( parentWidget() )
        parentWidget()->update();
}

ToolStripSectionLayout::ToolStripSectionLayout( const QString& title ) : QLayout(),
    m_titleText( title ),
    m_collapsed( false ),
    m_dirty( true )
{
    setContentsMargins( 0, 0, 0, 0 );
}

ToolStripSectionLayout::~ToolStripSectionLayout()
{
}

void ToolStripSectionLayout::addLayout( QLayout* layout )
{
    addChildLayout( layout );
    addItem( layout );
}

void ToolStripSectionLayout::drawSection( QPainter* painter, QWidget* widget )
{
    drawSeparator( m_separatorRect, painter, widget );

    widget->style()->drawItemText( painter, m_titleRect, Qt::AlignCenter, widget->palette(), true, m_titleText, QPalette::Text );
}

static void setWidgetsVisible( QLayoutItem* item, bool visible )
{
    QLayout* layout = item->layout();
    if ( layout ) {
        for ( int i = 0; i < layout->count(); i++ )
            setWidgetsVisible( layout->itemAt( i ), visible );
    }

    QWidget* widget = item->widget();
    if ( widget )
        widget->setVisible( visible );
}

void ToolStripSectionLayout::setCollapsed( bool collapsed )
{
    if ( m_collapsed != collapsed ) {
        m_collapsed = collapsed;
        setWidgetsVisible( this, !collapsed );
    }
}

Qt::Orientations ToolStripSectionLayout::expandingDirections() const
{
    return 0;
}

QSize ToolStripSectionLayout::sizeHint() const
{
    if ( m_dirty )
        const_cast<ToolStripSectionLayout*>( this )->calculateSize();
    return m_sizeHint;
}

QSize ToolStripSectionLayout::minimumSize() const
{
    return sizeHint();
}

QSize ToolStripSectionLayout::maximumSize() const
{
    return sizeHint();
}

static int calculateWidth( QLayoutItem* item )
{
    if ( item == NULL )
        return 0;

    QLayout* layout = item->layout();
    if ( layout ) {
        int width = 0;

        QGridLayout* gridLayout = qobject_cast<QGridLayout*>( layout );
        if ( gridLayout ) {
            for ( int column = 0; column < gridLayout->columnCount(); column++ ) {
                int maxWidth = 0;
                for ( int row = 0; row < gridLayout->rowCount(); row++ )
                    maxWidth = qMax( maxWidth, calculateWidth( gridLayout->itemAtPosition( row, column ) ) );
                width += maxWidth;
            }
        } else {
            for ( int i = 0; i < layout->count(); i++ )
                width += calculateWidth( layout->itemAt( i ) );
        }

        return width;
    }

    QWidget* widget = item->widget();
    if ( widget )
        return widget->sizeHint().width();

    return 0;
}

void ToolStripSectionLayout::calculateSize()
{
    QFontMetrics fontMetrics = parentWidget()->fontMetrics();

    QStyleOptionToolButton option;
    option.initFrom( parentWidget() );
    option.iconSize = QSize( 22, 22 );
    option.state |= QStyle::State_AutoRaise;
    option.subControls = QStyle::SC_ToolButton;
    option.toolButtonStyle = Qt::ToolButtonTextUnderIcon;

    QSize buttonSize = parentWidget()->style()->sizeFromContents( QStyle::CT_ToolButton, &option,
        QSize( 22, 22 + 4 + fontMetrics.height() * 2 ), parentWidget() );

    int width = calculateWidth( const_cast<ToolStripSectionLayout*>( this ) );

    m_titleSize = fontMetrics.size( 0, m_titleText );

    m_sizeHint = QSize( qMax( width, m_titleSize.width() ) + 9, buttonSize.height() + m_titleSize.height() + 9 );

    m_dirty = false;
}

int ToolStripSectionLayout::count() const
{
    return m_items.count();
}

QLayoutItem* ToolStripSectionLayout::itemAt( int index ) const
{
    return ( index >= 0 && index < m_items.count() ) ? m_items.at( index ) : NULL;
}

void ToolStripSectionLayout::addItem( QLayoutItem* item )
{
    m_items.append( item );
    invalidate();
}

QLayoutItem* ToolStripSectionLayout::takeAt( int index )
{
    QLayoutItem* item = m_items.takeAt( index );
    invalidate();
    return item;
}

void ToolStripSectionLayout::setGeometry( const QRect& rect )
{
    if ( !m_dirty && geometry() == rect )
        return;

    if ( m_dirty )
        calculateSize();

    QLayout::setGeometry( rect );

    int left = rect.left() + 3;

    foreach ( QLayoutItem* item, m_items ) {
        QSize size = item->sizeHint();
        item->setGeometry( QRect( left, rect.top() + 3, size.width(), rect.height() - m_titleSize.height() - 9 ) );
        left += size.width();
    }

    m_titleRect = QRect( rect.left() + 3, rect.bottom() - m_titleSize.height() - 2, rect.width() - 9, m_titleSize.height() );

    m_separatorRect = QRect( rect.right() - 2, rect.top(), 3, rect.height() );
}

void ToolStripSectionLayout::invalidate()
{
    m_dirty = true;
    QLayout::invalidate();
}

ChevronButton::ChevronButton( QWidget* parent ) : QToolButton( parent )
{
    setAutoRaise( true );
    setMenu( new QMenu( this ) );
    setPopupMode( QToolButton::InstantPopup );
    setFocusPolicy( Qt::NoFocus );

    QPixmap chevron = style()->standardPixmap( QStyle::SP_ToolBarHorizontalExtensionButton );

    setIcon( chevron );
    setIconSize( QSize( chevron.width(), 16 ) );
}

ChevronButton::~ChevronButton()
{
}

void ChevronButton::paintEvent( QPaintEvent* /*e*/ )
{
    QStylePainter painter( this );

    QStyleOptionToolButton option;
    initStyleOption( &option );

    option.features &= ~QStyleOptionToolButton::HasMenu;

    painter.drawComplexControl( QStyle::CC_ToolButton, option );
}

ActionButton::ActionButton( QWidget* parent ) : QToolButton( parent )
{
    setAutoRaise( true );
    setFocusPolicy( Qt::NoFocus );
}

ActionButton::~ActionButton()
{
}

void ActionButton::adjustText()
{
    if ( toolButtonStyle() == Qt::ToolButtonTextUnderIcon ) {
        QString text = defaultAction()->iconText();
        if ( !text.contains( '\n' ) ) {
            int mid = text.length() / 2;
            int after = text.indexOf( ' ', mid );
            int before = text.lastIndexOf( ' ', mid );
            if ( before < 0 && after < 0 )
                text.append( '\n' );
            else if ( after >= 0 && ( before < 0 || ( after - mid ) < ( mid - before ) ) )
                text.replace( after, 1, '\n' );
            else
                text.replace( before, 1, '\n' );
            setText( text );
        }
    }

    QKeySequence shortcut = defaultAction()->shortcut();
    if ( !shortcut.isEmpty() ) {
        setToolTip( QString( "%1 (%2)" ).arg( defaultAction()->toolTip(), shortcut.toString( QKeySequence::NativeText ) ) );
    } else if ( popupMode() == QToolButton::MenuButtonPopup && defaultAction()->menu() != NULL ) {
        QAction* defAction = defaultAction()->menu()->defaultAction();
        if ( defAction != NULL ) {
            shortcut = defAction->shortcut();
            if ( !shortcut.isEmpty() )
                setToolTip( QString( "%1 (%2)" ).arg( defaultAction()->toolTip(), shortcut.toString( QKeySequence::NativeText ) ) );
        }
    }
}

void ActionButton::actionEvent( QActionEvent* e )
{
    QToolButton::actionEvent( e );
    if ( e->type() == QEvent::ActionChanged && e->action() == defaultAction() )
        adjustText();
}
