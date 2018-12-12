#include "animationdockwidget.h"
#include <QStyleOption>
#include <QPainter>
#include "global.h"
#include "keysview.h"
#include "actionbutton.h"

AnimationDockWidget::AnimationDockWidget(QWidget *parent,
                                         KeysView *keysView) :
    QToolBar(parent) {
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    setIconSize(QSize(qRound(1.25*MIN_WIDGET_HEIGHT),
                      qRound(1.25*MIN_WIDGET_HEIGHT)));
    mLineButton = new ActionButton(
                ":/icons/node_line.png",
                "MAKE SEGMENT LINE", this);
    connect(mLineButton, &ActionButton::pressed,
            keysView, &KeysView::graphMakeSegmentsLinear);

    mCurveButton = new ActionButton(
                ":/icons/node_curve.png",
                "MAKE SEGMENT CURVE", this);
    connect(mCurveButton, &ActionButton::pressed,
            keysView, qOverload<>(&KeysView::graphMakeSegmentsSmooth));

    mSymmetricButton = new ActionButton(
                ":/icons/node_symmetric.png",
                "", this);
    connect(mSymmetricButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetSymmetricCtrl);

    mSmoothButton = new ActionButton(
                ":/icons/node_smooth.png",
                "", this);
    connect(mSmoothButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetSmoothCtrl);

    mCornerButton = new ActionButton(
                ":/icons/node_cusp.png",
                "", this);
    connect(mCornerButton, &ActionButton::pressed,
            keysView, &KeysView::graphSetCornerCtrl);

    mFitToHeightButton = new ActionButton(
                ":/icons/zoom.png",
                "", this);
    connect(mFitToHeightButton, &ActionButton::pressed,
            keysView, &KeysView::graphResetValueScaleAndMinShownAction);

//    mTwoSideCtrlButton = new ActionButton(
//                ":/icons/two_side_ctrl_white.png",
//                "", this);
// //    mTwoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                     QSizePolicy::Maximum);
//    connect(mTwoSideCtrlButton, &ActionButton::pressed,
//            keysView, &KeysView::graphSetTwoSideCtrlForSelected()) );

//    mLeftSideCtrlButton = new ActionButton(
//                ":/icons/left_side_ctrl_white.png",
//                "", this);
// //    mLeftSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                       QSizePolicy::Maximum);
//    connect(mLeftSideCtrlButton, &ActionButton::pressed,
//            keysView, &KeysView::graphSetLeftSideCtrlForSelected()) );

//    mRightSideCtrlButton = new ActionButton(
//                ":/icons/right_side_ctrl_white.png",
//                "", this);
// //    mRightSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                        QSizePolicy::Maximum);
//    connect(mRightSideCtrlButton, &ActionButton::pressed,
//            keysView, &KeysView::graphSetRightSideCtrlForSelected()) );

//    mNoSideCtrlButton = new ActionButton(
//                ":/icons/no_side_ctrl_white.png",
//                "", this);
// //    mNoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                     QSizePolicy::Maximum);
//    connect(mNoSideCtrlButton, &ActionButton::pressed,
//            keysView, &KeysView::graphSetNoSideCtrlForSelected()) );


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
