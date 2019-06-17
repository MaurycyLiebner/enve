#include "stackwidgetwrapper.h"
#include "GUI/widgetstack.h"

StackWidgetWrapper::StackWidgetWrapper(const SetupOp &setup,
                                       QWidget * const parent) :
    QWidget(parent), mSetupOp(setup) {
    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);
    mLayout->setSpacing(0);
    mLayout->setMargin(0);
    mSetupOp(this);
}

void StackWidgetWrapper::setMenuBar(StackWrapperMenu * const menu) {
    if(mMenuBar) delete mMenuBar;
    mMenuBar = menu;
    mLayout->insertWidget(0, mMenuBar);
    menu->setParent(this);
}

void StackWidgetWrapper::setCentralWidget(QWidget * const widget) {
    if(mCenterWidget) delete mCenterWidget;
    mCenterWidget = widget;
    mLayout->insertWidget(-1, mCenterWidget);
}

void StackWidgetWrapper::splitH() {
    split<HWidgetStack>();
}

void StackWidgetWrapper::splitV() {
    split<VWidgetStack>();
}

StackWrapperMenu::StackWrapperMenu() {
    setFixedHeight(MIN_WIDGET_DIM);
    QMenuBar * const actBar = new QMenuBar(this);
    actBar->setFixedHeight(MIN_WIDGET_DIM);
    mSplitV = new QAction("split v");
    mSplitH = new QAction("split h");
    mClose = new QAction("x");

    actBar->addAction(mSplitV);
    actBar->addAction(mSplitH);
    actBar->addAction(mClose);


    mMenuBarLayout = new QHBoxLayout(this);
    mMenuBarLayout->setSpacing(0);
    mMenuBarLayout->setMargin(0);
    mLayout = new QHBoxLayout();
    mMenuBarLayout->addLayout(mLayout);
//    mLayout->setContentsMargins(MIN_WIDGET_DIM, MIN_WIDGET_DIM/10,
//                                MIN_WIDGET_DIM/10, MIN_WIDGET_DIM);
    mMenuBarLayout->addWidget(actBar, 0, Qt::AlignRight);
    setLayout(mMenuBarLayout);

    setObjectName("menuBarWidget");
}

void StackWrapperMenu::addWidget(QWidget * const widget) {
    mLayout->addWidget(widget);
}

void StackWrapperMenu::setParent(StackWidgetWrapper * const parent) {
    if(mParent) {
        disconnect(mSplitV, &QAction::triggered,
                   mParent, &StackWidgetWrapper::splitV);
        disconnect(mSplitH, &QAction::triggered,
                   mParent, &StackWidgetWrapper::splitH);
        disconnect(mClose, &QAction::triggered,
                   mParent, &StackWidgetWrapper::deleteLater);
    }
    mParent = parent;
    if(mParent) {
        connect(mSplitV, &QAction::triggered,
                mParent, &StackWidgetWrapper::splitV);
        connect(mSplitH, &QAction::triggered,
                mParent, &StackWidgetWrapper::splitH);
        connect(mClose, &QAction::triggered,
                mParent, &StackWidgetWrapper::deleteLater);
    }
}
