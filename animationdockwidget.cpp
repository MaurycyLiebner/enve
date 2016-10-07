#include "animationdockwidget.h"
#include <QStyleOption>
#include "mainwindow.h"
#include "boxeslist.h"

AnimationDockWidget::AnimationDockWidget(QWidget *parent,
                                         BoxesList *boxesList) :
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    setStyleSheet(".AnimationDockWidget .QPushButton { background-color: black }");

    mButtonsLayout = new QHBoxLayout();
    mButtonsLayout->setMargin(0);
    mButtonsLayout->setSpacing(0);

    mSymmetricButton = new QPushButton(
                QIcon("pixmaps/icons/ink_node_symmetric_white.png"),
                "", this);
//    mSymmetricButton->setSizePolicy(QSizePolicy::Maximum,
//                                    QSizePolicy::Maximum);
    connect(mSymmetricButton, SIGNAL(pressed()),
            boxesList, SLOT(graphSetSymmetricCtrl()) );

    mSmoothButton = new QPushButton(
                QIcon("pixmaps/icons/ink_node_smooth_white.png"),
                "", this);
//    mSmoothButton->setSizePolicy(QSizePolicy::Maximum,
//                                 QSizePolicy::Maximum);
    connect(mSmoothButton, SIGNAL(pressed()),
            boxesList, SLOT(graphSetSmoothCtrl()) );

    mCornerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_node_cusp_white.png"),
                "", this);
//    mCornerButton->setSizePolicy(QSizePolicy::Maximum,
//                                 QSizePolicy::Maximum);
    connect(mCornerButton, SIGNAL(pressed()),
            boxesList, SLOT(graphSetCornerCtrl()) );

    mTwoSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/two_side_ctrl_white.png"),
                "", this);
//    mTwoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                     QSizePolicy::Maximum);
    connect(mTwoSideCtrlButton, SIGNAL(pressed()),
            boxesList, SLOT(graphSetTwoSideCtrlForSelected()) );

    mLeftSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/left_side_ctrl_white.png"),
                "", this);
//    mLeftSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                       QSizePolicy::Maximum);
    connect(mLeftSideCtrlButton, SIGNAL(pressed()),
            boxesList, SLOT(graphSetLeftSideCtrlForSelected()) );

    mRightSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/right_side_ctrl_white.png"),
                "", this);
//    mRightSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                        QSizePolicy::Maximum);
    connect(mRightSideCtrlButton, SIGNAL(pressed()),
            boxesList, SLOT(graphSetRightSideCtrlForSelected()) );

    mNoSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/no_side_ctrl_white.png"),
                "", this);
//    mNoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                     QSizePolicy::Maximum);
    connect(mNoSideCtrlButton, SIGNAL(pressed()),
            boxesList, SLOT(graphSetNoSideCtrlForSelected()) );

    mButtonsLayout->addWidget(mSymmetricButton);
    mButtonsLayout->addWidget(mSmoothButton);
    mButtonsLayout->addWidget(mCornerButton);
    mButtonsLayout->addWidget(mTwoSideCtrlButton);
    mButtonsLayout->addWidget(mLeftSideCtrlButton);
    mButtonsLayout->addWidget(mRightSideCtrlButton);
    mButtonsLayout->addWidget(mNoSideCtrlButton);

    setLayout(mButtonsLayout);
}

void AnimationDockWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.end();
}
