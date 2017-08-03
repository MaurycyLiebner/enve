#include "brushselector.h"
#include "../windowvariables.h"
#include <QGraphicsSceneWheelEvent>
#include <QtMath>

BrushSelector::BrushSelector(WindowVariables *window_vars_t) : QGraphicsItem()
{
    window_vars = window_vars_t;
    setBrushButtonDimensions(64, 6);
    setZValue(-1);
}

void BrushSelector::clearCurrentBrushButtons()
{
    for(int i = 0; i < n_brushes; i++)
    {
        current_collection->brushes.at(i)->hide();
    }
}

int BrushSelector::getButtonDim()
{
    return button_dim;
}

int BrushSelector::getButtonImgDim()
{
    return button_img_dim;
}

int BrushSelector::getButtonBorderW()
{
    return min_border;
}

void BrushSelector::wheelEvent(QGraphicsSceneWheelEvent *e)
{
    if(e->delta() > 0 && y() > 0)
    {
        return;
    }
    if(e->delta() < 0 && y() < -height)
    {
        return;
    }
    moveBy(0, e->delta()*0.5);
}

void BrushSelector::setBrushCollection(Collection *current_collection_t)
{
    clearCurrentBrushButtons();
    current_collection = current_collection_t;
    n_brushes = current_collection->brushes.count();
    updateDisplayedBrushes();
}

void BrushSelector::updateDisplayedBrushes()
{
    int curr_row = 0;
    int curr_col = 0;
    bool darker_t = false;
    bool darker_row_t = false;
    for(int i = 0; i < n_brushes; i++)
    {
        BrushButton *button_t = current_collection->brushes.at(i);
        button_t->setX(curr_col*button_dim);// + margin);
        button_t->setY(curr_row*button_dim);
        button_t->show();
        button_t->setDarker(darker_t);
        darker_t = !darker_t;
        curr_col++;
        if(curr_col >= n_columns)
        {
            curr_col = 0;
            curr_row++;
            darker_row_t = !darker_row_t;
            darker_t = darker_row_t;
        }
    }
}

void BrushSelector::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{

}

QRectF BrushSelector::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

void BrushSelector::setWidth(int width_t)
{
    width = width_t;
    n_columns = width/(button_img_dim + min_border);
    button_dim = qCeil(width/(float)n_columns);
    int n_rows = qCeil(n_brushes/(float)n_columns);
    height = button_dim*n_rows;
    updateDisplayedBrushes();
}

void BrushSelector::setBrushButtonDimensions(int button_img_dim_t, int button_border_t)
{
    button_img_dim = button_img_dim_t;
    button_dim = button_img_dim_t + button_border_t;
    min_border = button_border_t;
    updateDisplayedBrushes();
}

void BrushSelector::selectBrush(BrushButton *button_t)
{
    if(current_button != NULL)
    {
        current_button->deselect();
    }
    current_button = button_t;
    window_vars->setBrushButton(button_t);
}
