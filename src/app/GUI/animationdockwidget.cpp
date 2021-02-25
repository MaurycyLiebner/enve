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

#include "animationdockwidget.h"
#include <QStyleOption>
#include <QPainter>
#include "GUI/global.h"
#include "keysview.h"
#include "actionbutton.h"
#include "switchbutton.h"
#include "Private/esettings.h"

AnimationDockWidget::AnimationDockWidget(QWidget *parent,
                                         KeysView *keysView) :
    QToolBar(parent) {
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    eSizesUI::widget.add(this, [this](const int size) {
        setIconSize(QSize(qRound(1.25*size), qRound(1.25*size)));
    });

    const QString iconsDir = eSettings::sIconsDir() + "/toolbarButtons";

    mLineButton = new ActionButton("toolbarButtons/segmentLine.png",
                                   "Make Segment Line", this);
    connect(mLineButton, &ActionButton::pressed,
            keysView, &KeysView::graphMakeSegmentsLinearAction);

    mCurveButton = new ActionButton("toolbarButtons/segmentCurve.png",
                                    "Make Segment Curve", this);
    connect(mCurveButton, &ActionButton::pressed,
            keysView, qOverload<>(&KeysView::graphMakeSegmentsSmoothAction));

    mSymmetricButton = new ActionButton("toolbarButtons/nodeSymmetric.png",
                                        "Symmetric Nodes", this);
    connect(mSymmetricButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetSymmetricCtrlAction);

    mSmoothButton = new ActionButton("toolbarButtons/nodeSmooth.png",
                                     "Smooth Nodes", this);
    connect(mSmoothButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetSmoothCtrlAction);

    mCornerButton = new ActionButton("toolbarButtons/nodeCorner.png",
                                     "Corner Nodes", this);
    connect(mCornerButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetCornerCtrlAction);

    mFitToHeightButton = new ActionButton("toolbarButtons/zoom.png",
                                          "Fit Vertical", this);
    connect(mFitToHeightButton, &ActionButton::pressed,
            keysView, &KeysView::graphResetValueScaleAndMinShownAction);

    const auto valueLines = SwitchButton::sCreate2Switch(
                                "toolbarButtons/horizontalLinesChecked.png",
                                "toolbarButtons/horizontalLinesUnchecked.png",
                                gSingleLineTooltip("Show/Hide Value Lines"),
                                this);
    connect(valueLines, &SwitchButton::toggled,
            keysView, &KeysView::graphSetValueLinesDisabled);

    const auto selectedVisible = SwitchButton::sCreate2Switch(
                                     "toolbarButtons/onlySelectedVisibleUnchecked.png",
                                     "toolbarButtons/onlySelectedVisibleChecked.png",
                                     gSingleLineTooltip("View Only Selected Objects' Properties"),
                                     this);
    connect(selectedVisible, &SwitchButton::toggled,
            keysView, &KeysView::graphSetOnlySelectedVisible);

    addWidget(mLineButton);
    addWidget(mCurveButton);
    addSeparator();
    addWidget(mSymmetricButton);
    addWidget(mSmoothButton);
    addWidget(mCornerButton);
    addSeparator();
    addWidget(mFitToHeightButton);
    addWidget(valueLines);
    addSeparator();
    addWidget(selectedVisible);

    setStyleSheet("QToolBar {"
                      "padding: 10px;"
                      "margin-bottom: -1px;"
                      "border: 3px solid rgb(25, 25, 25);"
                  "}");
}

void AnimationDockWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.end();
}
