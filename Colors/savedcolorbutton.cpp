#include "savedcolorbutton.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include "global.h"

SavedColorButton::SavedColorButton(Color color_t, QWidget *parent) : QWidget(parent)
{
    color = color_t;
    setFixedSize(2*MIN_WIDGET_HEIGHT, 2*MIN_WIDGET_HEIGHT);
}

void SavedColorButton::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        emit colorButtonPressed(color);
    }
    else if(e->button() == Qt::RightButton)
    {
        QMenu menu(this);
        menu.addAction("Delete Color");
        QAction *selected_action = menu.exec(e->globalPos());
        if(selected_action != nullptr)
        {
            if(selected_action->text() == "Delete Color")
            {
                deleteLater();
            }
        }
        else
        {

        }
    }
}

void SavedColorButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), color.qcol);
    p.end();
}

