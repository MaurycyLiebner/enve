// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "savedcolorswidget.h"
#include "savedcolorbutton.h"
#include <QAction>
#include <QMenu>
#include "colorhelpers.h"

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
