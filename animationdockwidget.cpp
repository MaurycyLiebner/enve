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

    setIconSize(QSize(1.25*MIN_WIDGET_HEIGHT,
                      1.25*MIN_WIDGET_HEIGHT));
    mSymmetricButton = new ActionButton(
                ":/icons/node_symmetric.png",
                "", this);
//    mSymmetricButton->setSizePolicy(QSizePolicy::Maximum,
//                                    QSizePolicy::Maximum);
    connect(mSymmetricButton, SIGNAL(pressed()),
            keysView, SLOT(graphSetSymmetricCtrl()) );

    mSmoothButton = new ActionButton(
                ":/icons/node_smooth.png",
                "", this);
//    mSmoothButton->setSizePolicy(QSizePolicy::Maximum,
//                                 QSizePolicy::Maximum);
    connect(mSmoothButton, SIGNAL(pressed()),
            keysView, SLOT(graphSetSmoothCtrl()) );

    mCornerButton = new ActionButton(
                ":/icons/node_cusp.png",
                "", this);
//    mCornerButton->setSizePolicy(QSizePolicy::Maximum,
//                                 QSizePolicy::Maximum);
    connect(mCornerButton, SIGNAL(pressed()),
            keysView, SLOT(graphSetCornerCtrl()) );

    mFitToHeightButton = new ActionButton(
                ":/icons/zoom.png",
                "", this);
//    mCornerButton->setSizePolicy(QSizePolicy::Maximum,
//                                 QSizePolicy::Maximum);
    connect(mFitToHeightButton, SIGNAL(pressed()),
            keysView, SLOT(graphResetValueScaleAndMinShownAction()) );

//    mTwoSideCtrlButton = new ActionButton(
//                ":/icons/two_side_ctrl_white.png",
//                "", this);
// //    mTwoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                     QSizePolicy::Maximum);
//    connect(mTwoSideCtrlButton, SIGNAL(pressed()),
//            keysView, SLOT(graphSetTwoSideCtrlForSelected()) );

//    mLeftSideCtrlButton = new ActionButton(
//                ":/icons/left_side_ctrl_white.png",
//                "", this);
// //    mLeftSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                       QSizePolicy::Maximum);
//    connect(mLeftSideCtrlButton, SIGNAL(pressed()),
//            keysView, SLOT(graphSetLeftSideCtrlForSelected()) );

//    mRightSideCtrlButton = new ActionButton(
//                ":/icons/right_side_ctrl_white.png",
//                "", this);
// //    mRightSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                        QSizePolicy::Maximum);
//    connect(mRightSideCtrlButton, SIGNAL(pressed()),
//            keysView, SLOT(graphSetRightSideCtrlForSelected()) );

//    mNoSideCtrlButton = new ActionButton(
//                ":/icons/no_side_ctrl_white.png",
//                "", this);
// //    mNoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
// //                                     QSizePolicy::Maximum);
//    connect(mNoSideCtrlButton, SIGNAL(pressed()),
//            keysView, SLOT(graphSetNoSideCtrlForSelected()) );
    mActionLine = new ActionButton(
                ":/icons/node_line.png",
                "MAKE SEGMENT LINE", this);
    addWidget(mActionLine);

    mActionCurve = new ActionButton(
                ":/icons/node_curve.png",
                "MAKE SEGMENT CURVE", this);
    addWidget(mActionCurve);


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
