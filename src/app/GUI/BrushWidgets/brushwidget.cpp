// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "brushwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>

BrushWidget::BrushWidget(BrushContexedWrapper * const brushCWrapper,
                         QWidget * const parent) :
    QWidget(parent), mBrushCWrapper(brushCWrapper) {
    Q_ASSERT(brushCWrapper);
    setFixedSize(64, 64);
    connect(brushCWrapper, &BrushContexedWrapper::selectionChanged,
            this, &BrushWidget::selectionChanged);
    connect(brushCWrapper, &BrushContexedWrapper::bookmarkedChanged,
            this, qOverload<>(&BrushWidget::update));
}

void BrushWidget::paintEvent(QPaintEvent *) {
    if(!mBrushCWrapper) return;
    QPainter p(this);

    const auto& brushData = mBrushCWrapper->getBrushData();
    p.drawImage(QRect(0, 0, width(), height()), brushData.fIcon);

    if(mBrushCWrapper->bookmarked()) {
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBrush(QColor(0, 175, 255));
        p.setPen(Qt::NoPen);
        p.drawEllipse(5, 5, 7, 7);
        p.setBrush(Qt::NoBrush);
        p.setRenderHint(QPainter::Antialiasing, false);
    }

    if(isSelected()) {
        if(mHovered) {
            p.setPen(QPen(Qt::red, 1, Qt::DashLine));
            p.drawRect(2, 2, width() - 5, height() - 5);
        }
        p.setPen(QPen(Qt::red, 2));
        p.drawRect(1, 1, width() - 2, height() - 2);
    } else if(mHovered) {
        p.setPen(QPen(Qt::red, 1, Qt::DashLine));
        p.drawRect(0, 0, width() - 1, height() - 1);
    }

    p.end();
}

void BrushWidget::mousePressEvent(QMouseEvent *e) {
    if(!mBrushCWrapper) return;
    const auto pressedButton = e->button();
    if(pressedButton == Qt::RightButton) {
        QMenu menu(this);

        if(mBrushCWrapper->bookmarked()) menu.addAction("Unbookmark");
        else menu.addAction("Bookmark");
        const auto act = menu.exec(e->globalPos());
        if(act) {
            if(act->text() == "Bookmark") {
                mBrushCWrapper->bookmark();
            } else if(act->text() == "Unbookmark") {
                mBrushCWrapper->unbookmark();
            }
        }
    } else if(pressedButton == Qt::LeftButton) {
        mBrushCWrapper->setSelected(true);
        emit triggered(mBrushCWrapper);
    } else return;
    update();
}
