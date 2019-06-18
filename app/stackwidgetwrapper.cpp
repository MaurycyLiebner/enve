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

void StackWidgetWrapper::closeWrapper() {
    const auto vStack = dynamic_cast<VWidgetStack*>(parentWidget());
    if(vStack) {
        vStack->takeWidget(this);
        if(vStack->count() == 1) {
            const auto wid = vStack->takeWidget(0);
            bool central = false;
            gReplaceWidget(vStack, wid, &central);
            vStack->deleteLater();
            if(central) {
                const auto sww = dynamic_cast<StackWidgetWrapper*>(wid);
                if(sww) sww->disableClose();
            }
        }
    } else {
        const auto hStack = dynamic_cast<HWidgetStack*>(parentWidget());
        if(hStack) {
            hStack->takeWidget(this);
            if(hStack->count() == 1) {
                const auto wid = hStack->takeWidget(0);
                bool central = false;
                gReplaceWidget(hStack, wid, &central);
                hStack->deleteLater();
                if(central) {
                    const auto sww = dynamic_cast<StackWidgetWrapper*>(wid);
                    if(sww) sww->disableClose();
                }
            }
        }
    }

    deleteLater();
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
                mParent, &StackWidgetWrapper::closeWrapper);
    }
}
