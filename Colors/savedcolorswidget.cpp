#include "savedcolorswidget.h"
#include "savedcolorbutton.h"
#include <QAction>
#include <QMenu>
#include "color.h"

SavedColorsWidget::SavedColorsWidget(QWidget *parent)
    : QWidget(parent)
{
    main_layout = new QHBoxLayout(this);
    main_layout->setAlignment(Qt::AlignLeft);
    setLayout(main_layout);
}

void SavedColorsWidget::addColorButton(Color color_t)
{
    SavedColorButton *button_t = new SavedColorButton(color_t, this);
    connect(button_t, SIGNAL(colorButtonPressed(Color)), this, SLOT(setColorFromButton(Color) ) );
    main_layout->addWidget(button_t, Qt::AlignLeft);
}

void SavedColorsWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton)
    {
        QMenu menu(this);
        menu.addAction("Add Color");
        menu.addAction("Delete Collection");
        QAction *selected_action = menu.exec(e->globalPos());
        if(selected_action != NULL)
        {
            if(selected_action->text() == "Add Color")
            {
                //addColorButton(window_variables->current_color);
            }
            else if(selected_action->text() == "Delete Collection")
            {

            }
        }
        else
        {

        }
    }
}

void SavedColorsWidget::setColorFromButton(const Color &color_t) {

}
