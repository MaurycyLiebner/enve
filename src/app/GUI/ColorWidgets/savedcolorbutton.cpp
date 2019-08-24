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

#include "savedcolorbutton.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include "GUI/global.h"

SavedColorButton::SavedColorButton(const QColor &colorT,
                                   QWidget *parent) :
    QWidget(parent) {
    mColor = colorT;
    setFixedSize(2*MIN_WIDGET_DIM, 2*MIN_WIDGET_DIM);
}

void SavedColorButton::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton) {
        emit colorButtonPressed(mColor);
    } else if(e->button() == Qt::RightButton) {
        QMenu menu(this);
        menu.addAction("Delete Color");
        QAction *selected_action = menu.exec(e->globalPos());
        if(selected_action != nullptr) {
            if(selected_action->text() == "Delete Color") {
                deleteLater();
            }
        } else {

        }
    }
}

void SavedColorButton::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), mColor);
    p.end();
}

