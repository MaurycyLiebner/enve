#include "animationdockwidget.h"
#include <QStyleOption>
#include <QPainter>
#include "GUI/global.h"
#include "keysview.h"
#include "actionbutton.h"
#include "esettings.h"

AnimationDockWidget::AnimationDockWidget(QWidget *parent,
                                         KeysView *keysView) :
    QToolBar(parent) {
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    setIconSize(QSize(qRound(1.25*MIN_WIDGET_DIM),
                      qRound(1.25*MIN_WIDGET_DIM)));

    const QString iconsDir = eSettings::sIconsDir() + "/toolbarButtons";

    mLineButton = new ActionButton(iconsDir + "/segmentLine.png",
                                   "MAKE SEGMENT LINE", this);
    connect(mLineButton, &ActionButton::pressed,
            keysView, &KeysView::graphMakeSegmentsLinearAction);

    mCurveButton = new ActionButton(iconsDir + "/segmentCurve.png",
                                    "MAKE SEGMENT CURVE", this);
    connect(mCurveButton, &ActionButton::pressed,
            keysView, qOverload<>(&KeysView::graphMakeSegmentsSmoothAction));

    mSymmetricButton = new ActionButton(iconsDir + "/nodeSymmetric.png", "", this);
    connect(mSymmetricButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetSymmetricCtrlAction);

    mSmoothButton = new ActionButton(iconsDir + "/nodeSmooth.png", "", this);
    connect(mSmoothButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetSmoothCtrlAction);

    mCornerButton = new ActionButton(iconsDir + "/nodeCorner.png", "", this);
    connect(mCornerButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetCornerCtrlAction);

    mFitToHeightButton = new ActionButton(iconsDir + "/zoom.png", "", this);
    connect(mFitToHeightButton, &ActionButton::pressed,
            keysView, &KeysView::graphResetValueScaleAndMinShownAction);

    addWidget(mLineButton);
    addWidget(mCurveButton);
    addSeparator();
    addWidget(mSymmetricButton);
    addWidget(mSmoothButton);
    addWidget(mCornerButton);
    addSeparator();
    addWidget(mFitToHeightButton);
//    mButtonsLayout->addWidget(mTwoSideCtrlButton);
//    mButtonsLayout->addWidget(mLeftSideCtrlButton);
//    mButtonsLayout->addWidget(mRightSideCtrlButton);
//    mButtonsLayout->addWidget(mNoSideCtrlButton);

    setStyleSheet("border: 1px solid black; padding: 10px; margin-bottom: -1px");
}

void AnimationDockWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.end();
}
