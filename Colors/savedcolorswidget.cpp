#include "savedcolorswidget.h"
#include "savedcolorbutton.h"
#include <QAction>
#include <QMenu>
#include "helpers.h"

SavedColorsWidget::SavedColorsWidget(QWidget *parent)
    : QWidget(parent) {
    mMainLayout = new QHBoxLayout(this);
    mMainLayout->setAlignment(Qt::AlignLeft);
    setLayout(mMainLayout);
}

void SavedColorsWidget::addColorButton(const QColor& colorT) {
    SavedColorButton *buttonT = new SavedColorButton(colorT, this);
    connect(buttonT, SIGNAL(colorButtonPressed(QColor)),
            this, SLOT(setColorFromButton(QColor) ) );
    mMainLayout->addWidget(buttonT, Qt::AlignLeft);
}

void SavedColorsWidget::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::RightButton) {
        QMenu menu(this);
        menu.addAction("Add Color");
        menu.addAction("Delete Collection");
        QAction *selected_action = menu.exec(e->globalPos());
        if(selected_action != nullptr) {
            if(selected_action->text() == "Add Color") {
                //addColorButton(window_variables->current_color);
            } else if(selected_action->text() == "Delete Collection") {

            }
        } else {

        }
    }
}

void SavedColorsWidget::setColorFromButton(const QColor &colorT) {
    Q_UNUSED(colorT)
}
