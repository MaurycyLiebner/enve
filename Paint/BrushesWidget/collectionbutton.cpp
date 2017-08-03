#include "collectionbutton.h"
#include "brushcollectionselector.h"
#include "brushbutton.h"
#include <QFontMetrics>
#include <QPainter>
#include "mainwindow.h"

CollectionButton::CollectionButton(QString folder_path, BrushSelector *brush_selector_t,
                                   BrushCollectionSelector *parent_t) : QGraphicsItem(parent_t)
{
    brush_collection_selector = parent_t;
    collection.ini(folder_path, brush_selector_t);
    setAcceptHoverEvents(true);
    updateTextWidth();
}

void CollectionButton::select()
{
    setSelected(true);
}

void CollectionButton::deselect()
{
    setSelected(false);
}

void CollectionButton::setSelected(bool selected_t)
{
    if(selected == selected_t)
    {
        return;
    }
    if(selected_t)
    {
        brush_collection_selector->selectCollection(this);
    }
    selected = selected_t;
    update();
}

void CollectionButton::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
//    QColor col;
//    if(darker)
//    {
//        if(selected)
//        {
//            if(hover)
//            {
//                col = MainWindow::color_selected_hover_darker.qcol;
//            }
//            else
//            {
//                col = MainWindow::color_selected_darker.qcol;
//            }
//        }
//        else
//        {
//            if(hover)
//            {
//                col = MainWindow::color_hover_darker.qcol;
//            }
//            else
//            {
//                col = MainWindow::color_darker.qcol;
//            }
//        }
//    }
//    else
//    {
//        if(selected)
//        {
//            if(hover)
//            {
//                col = MainWindow::color_selected_hover.qcol;
//            }
//            else
//            {
//                col = MainWindow::color_selected.qcol;
//            }
//        }
//        else
//        {
//            if(hover)
//            {
//                col = MainWindow::color_hover.qcol;
//            }
//            else
//            {
//                col = MainWindow::color_light.qcol;
//            }
//        }
//    }
//    p->fillRect(boundingRect(), col);
    p->drawText(boundingRect(), Qt::AlignCenter, collection.name);
}

QRectF CollectionButton::boundingRect() const
{
    return QRectF(0, 0, width, brush_collection_selector->getButtonHeight());
}

void CollectionButton::setAdditionalMargin(int add_margin_t)
{
    add_margin = add_margin_t;
    updateWidth();
}

void CollectionButton::updateWidth()
{
    width = text_width + 2*brush_collection_selector->getButtonMargin() + add_margin;
}

void CollectionButton::mousePressEvent(QGraphicsSceneMouseEvent *)
{

}

void CollectionButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    select();
}

void CollectionButton::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    hover = true;
    update();
}

void CollectionButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    hover = false;
    update();
}

int CollectionButton::getWidth()
{
    return width;
}

Collection *CollectionButton::getCollection()
{
    return &collection;
}

bool CollectionButton::isDarker()
{
    return darker;
}

void CollectionButton::setDarker(bool darker_t)
{
    darker = darker_t;
    update();
}

void CollectionButton::updateTextWidth()
{
    QFontMetrics fm(brush_collection_selector->getButtonFont());
    text_width = fm.width(collection.name);
}

int CollectionButton::getTextWidth()
{
    return text_width;
}

