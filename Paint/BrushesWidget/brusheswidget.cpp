#include "brusheswidget.h"
#include "brushselector.h"
#include "brushcollectionselector.h"
#include <QResizeEvent>
#include "../windowvariables.h"

BrushesWidget::BrushesWidget(WindowVariables *window_vars_t,
                             QWidget *parent) : QGraphicsView(parent) {
    window_vars = window_vars_t;
    graphics_scene = new QGraphicsScene(this);
    setScene(graphics_scene);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    brush_selector = new BrushSelector(window_vars);
    graphics_scene->addItem(brush_selector);
    brush_selector->setPos(0, 0);
    brush_collection_selector = new BrushCollectionSelector(brush_selector);
    brush_collection_selector->setPos(0, 0);
    graphics_scene->addItem(brush_collection_selector);
    brush_collection_selector->loadCollections("brushes/");
}

void BrushesWidget::resizeEvent(QResizeEvent *e)
{
    int new_width = e->size().width();
    setSceneRect(0, 0, new_width, e->size().height());
    brush_selector->setWidth(new_width);
    brush_collection_selector->setWidth(new_width);
}

