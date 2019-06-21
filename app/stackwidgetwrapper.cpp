#include "stackwidgetwrapper.h"
#include "GUI/widgetstack.h"

StackWidgetWrapper::StackWidgetWrapper(WidgetStackLayoutItem* const layoutItem,
                                       const LayoutItemCreator &layoutItemCreator,
                                       const Creator &creator,
                                       const SetupOp &setup,
                                       QWidget* const parent) :
    QWidget(parent), mLayoutItem(layoutItem),
    mLayoutItemCreator(layoutItemCreator),
    mCreator(creator), mSetupOp(setup) {
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

StackWidgetWrapper* StackWidgetWrapper::splitH() {
    auto otherLayoutItem = mLayoutItemCreator();
    const auto otherLayoutItemPtr = otherLayoutItem.get();
    mLayoutItem->splitH(std::move(otherLayoutItem));
    return split<HWidgetStack>(otherLayoutItemPtr);
}

StackWidgetWrapper* StackWidgetWrapper::splitV() {
    auto otherLayoutItem = mLayoutItemCreator();
    const auto otherLayoutItemPtr = otherLayoutItem.get();
    mLayoutItem->splitV(std::move(otherLayoutItem));
    return split<VWidgetStack>(otherLayoutItemPtr);
}

void StackWidgetWrapper::closeWrapper() {
    mLayoutItem->close();
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

void StackWidgetWrapper::disableClose() {
    if(mMenuBar) mMenuBar->disableClose();
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

template<class SplitItemClass>
void SplittableStackItem::split(WidgetPtr &&other) {
    Q_ASSERT(mParent);
    SplitPtr vStack = std::make_unique<SplitItemClass>();
    const auto vStackPtr = vStack.get();
    UniPtr thisUni = mParent->replaceChild(this, std::move(vStack));
    vStackPtr->setChildren(std::move(thisUni), std::move(other));
}

template<class SplitItemClass>
StackLayoutItem::SplitPtr SplittableStackItem::split(WidgetPtr &&thisUni,
                                                     WidgetPtr &&other) {
    SplitPtr vStack = std::make_unique<SplitItemClass>();
    vStack->setChildren(std::move(thisUni), std::move(other));
    return vStack;
}

void SplittableStackItem::splitV(WidgetPtr &&other) {
    split<VSplitStackItem>(std::move(other));
}

StackLayoutItem::SplitPtr SplittableStackItem::splitV(WidgetPtr &&thisUni,
                                                      WidgetPtr &&other) {
    return split<VSplitStackItem>(std::move(thisUni), std::move(other));
}

void SplittableStackItem::splitH(WidgetPtr &&other) {
    split<HSplitStackItem>(std::move(other));
}

StackLayoutItem::SplitPtr SplittableStackItem::splitH(WidgetPtr &&thisUni,
                                                      WidgetPtr &&other) {
    return split<HSplitStackItem>(std::move(thisUni), std::move(other));
}
