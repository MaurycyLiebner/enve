#include "stackwidgetwrapper.h"
#include "GUI/widgetstack.h"

StackWidgetWrapper::StackWidgetWrapper(const SetupOp &setup,
                                       QWidget * const parent) :
    QWidget(parent), mSetupOp(setup) {
    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);

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
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    QMenuBar * const actBar = new QMenuBar(this);
    mSplitV = new QAction("split v");
    mSplitH = new QAction("split h");
    mClose = new QAction("x");

    actBar->addAction(mSplitV);
    actBar->addAction(mSplitH);
    actBar->addAction(mClose);
    setCornerWidget(actBar);
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
