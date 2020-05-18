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

#include "alignwidget.h"

#include <QLabel>

#include "actionbutton.h"
#include "GUI/global.h"

AlignWidget::AlignWidget(QWidget* const parent) :
    QWidget(parent) {
    const auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    const auto combosLay = new QHBoxLayout;
    eSizesUI::widget.addHalfSpacing(mainLayout);
    mainLayout->addLayout(combosLay);

    combosLay->addWidget(new QLabel("Align: "));
    mAlignPivot = new QComboBox(this);
    mAlignPivot->addItem("Geometry");
    mAlignPivot->addItem("Pivot");
    combosLay->addWidget(mAlignPivot);

    combosLay->addWidget(new QLabel(" Relative to: "));
    mRelativeTo = new QComboBox(this);
    mRelativeTo->addItem("Scene");
    mRelativeTo->addItem("Last Selected");
    combosLay->addWidget(mRelativeTo);

    const auto buttonsLay = new QHBoxLayout;
    eSizesUI::widget.addHalfSpacing(mainLayout);
    mainLayout->addLayout(buttonsLay);
    eSizesUI::widget.addHalfSpacing(mainLayout);

    const auto leftButton = new ActionButton(
                                "toolbarButtons/pivot-align-left.png",
                                gSingleLineTooltip("Align Left"),
                                this);
    connect(leftButton, &ActionButton::pressed, this, [this]() {
        triggerAlign(Qt::AlignLeft);
    });
    buttonsLay->addWidget(leftButton);

    const auto hCenterButton = new ActionButton(
                                   "toolbarButtons/pivot-align-hcenter.png",
                                   gSingleLineTooltip("Align Horizontal Center"),
                                   this);
    connect(hCenterButton, &ActionButton::pressed, this, [this]() {
        triggerAlign(Qt::AlignHCenter);
    });
    buttonsLay->addWidget(hCenterButton);

    const auto rightButton = new ActionButton(
                                 "toolbarButtons/pivot-align-right.png",
                                 gSingleLineTooltip("Align Right"),
                                 this);
    connect(rightButton, &ActionButton::pressed, this, [this]() {
        triggerAlign(Qt::AlignRight);
    });
    buttonsLay->addWidget(rightButton);

    const auto line0 = new QFrame;
    line0->setFrameShape(QFrame::VLine);
    line0->setFrameShadow(QFrame::Sunken);
    buttonsLay->addWidget(line0);

    const auto topButton = new ActionButton(
                                "toolbarButtons/pivot-align-top.png",
                                gSingleLineTooltip("Align Top"),
                                this);
    connect(topButton, &ActionButton::pressed, this, [this]() {
        triggerAlign(Qt::AlignTop);
    });
    buttonsLay->addWidget(topButton);

    const auto vCenterButton = new ActionButton(
                                   "toolbarButtons/pivot-align-vcenter.png",
                                   gSingleLineTooltip("Align Vertical Center"),
                                   this);
    connect(vCenterButton, &ActionButton::pressed, this, [this]() {
        triggerAlign(Qt::AlignVCenter);
    });
    buttonsLay->addWidget(vCenterButton);

    const auto bottomButton = new ActionButton(
                                  "toolbarButtons/pivot-align-bottom.png",
                                  gSingleLineTooltip("Align Bottom"),
                                  this);
    connect(bottomButton, &ActionButton::pressed, this, [this]() {
        triggerAlign(Qt::AlignBottom);
    });
    buttonsLay->addWidget(bottomButton);
}

void AlignWidget::triggerAlign(const Qt::Alignment align) {
    const auto alignPivot = static_cast<AlignPivot>(
                                mAlignPivot->currentIndex());
    const auto relativeTo = static_cast<AlignRelativeTo>(
                                mRelativeTo->currentIndex());
    emit alignTriggered(align, alignPivot, relativeTo);
}
