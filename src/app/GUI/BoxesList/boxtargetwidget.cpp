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

#include "boxtargetwidget.h"
#include <QMimeData>
#include <QPainter>
#include <QMenu>
#include "Boxes/containerbox.h"
#include "Properties/boxtargetproperty.h"
#include "GUI/mainwindow.h"
#include "Properties/emimedata.h"

BoxTargetWidget::BoxTargetWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
    setMaximumWidth(150);
}

void BoxTargetWidget::setTargetProperty(BoxTargetProperty *property) {
    auto& conn = mProperty.assign(property);
    if(property) conn << connect(property, &BoxTargetProperty::targetSet,
                                 this, qOverload<>(&QWidget::update));
    update();
}

void BoxTargetWidget::dropEvent(QDropEvent *event) {
    if(eMimeData::sHasType<BoundingBox>(event->mimeData())) {
        auto boxMimeData = static_cast<const eMimeData*>(event->mimeData());
        BoundingBox *targetT = boxMimeData->getObjects<BoundingBox>().first();
        mProperty->setTarget(targetT);
        mDragging = false;
        update();
        Document::sInstance->actionFinished();
    }
}

void BoxTargetWidget::dragEnterEvent(QDragEnterEvent *event) {
    if(eMimeData::sHasType<BoundingBox>(event->mimeData())) {
        auto boxMimeData = static_cast<const eMimeData*>(event->mimeData());
        BoundingBox *targetT = boxMimeData->getObjects<BoundingBox>().first();
        const auto tester = [](Property * prop) {
            return enve_cast<BoundingBox*>(prop);
        };
        const auto parentBox = mProperty->getFirstAncestor<BoundingBox>(tester);
        Q_ASSERT(parentBox);
        Q_ASSERT(targetT);
        if(parentBox == targetT) return;
        if(parentBox->getParentGroup() != targetT->getParentGroup()) return;
        event->acceptProposedAction();
        mDragging = true;
        update();
    }
}

void BoxTargetWidget::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

void BoxTargetWidget::dragLeaveEvent(QDragLeaveEvent *event) {
    mDragging = false;
    update();
    event->accept();
}

void BoxTargetWidget::mousePressEvent(QMouseEvent *event) {
    if(!mProperty) return;
    if(event->button() == Qt::LeftButton) {
        auto tester = &Property::SWT_isBoundingBox;
        BoundingBox *parentBox =
                mProperty->getFirstAncestor<BoundingBox>(tester);
        if(!parentBox) return;
        const auto srcGroup = parentBox->getParentGroup();
        if(!srcGroup) return;
        const auto& boxesT = srcGroup->getContainedBoxes();
        QMenu menu(this);


        BoundingBox *currentTarget = mProperty->getTarget();
        int i = -1;
        QAction *act = menu.addAction("-none-");
        if(!currentTarget) {
            act->setCheckable(true);
            act->setChecked(true);
            act->setDisabled(true);
        }
        for(const auto& boxT : boxesT) {
            i++;
            if(boxT == parentBox) continue;
            QAction *act2 = menu.addAction(boxT->prp_getName());
            act2->setProperty("targetBoxPtr", i);
            if(currentTarget == boxT) {
                act2->setCheckable(true);
                act2->setChecked(true);
                act2->setDisabled(true);
            }
        }
        QAction *selected_action = menu.exec(mapToGlobal(QPoint(0, height())));
        if(selected_action != nullptr) {
            QVariant varT = selected_action->property("targetBoxPtr");
            if(varT.isValid()) {
                mProperty->setTarget(boxesT.at(varT.toInt()));
            } else {
                mProperty->setTarget(nullptr);
            }
            Document::sInstance->actionFinished();
        } else {

        }
    } else if(event->button() == Qt::RightButton) {

    }
}

void BoxTargetWidget::paintEvent(QPaintEvent *) {
    if(!mProperty) return;
    QPainter p(this);
    if(mProperty->SWT_isDisabled()) p.setOpacity(.5);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::white);
    if(mDragging) {
        p.setPen(Qt::white);
    } else {
        p.setPen(Qt::black);
    }
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5., 5.);

    p.setPen(Qt::black);
    if(mProperty) {
        BoundingBox *target = mProperty->getTarget();
        if(!target) {
            p.drawText(rect(), Qt::AlignCenter, "-none-");
        } else {
            p.drawText(rect(), Qt::AlignCenter, target->prp_getName());
        }
    }

    p.end();
}
