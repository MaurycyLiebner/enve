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
    menu->setTarget(this);
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
    setStyleSheet("QMenuBar { border-bottom: 1px solid black; }");

    mCornerMenu = new StackWrapperCornerMenu();
    setCornerWidget(mCornerMenu);
}

void StackWrapperMenu::setTarget(StackWidgetWrapper * const target) {
    mCornerMenu->setTarget(target);
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

void ParentStackLayoutItem::sWriteChild(
        StackLayoutItem * const child, QIODevice * const dst) {
    child->writeType(dst);
    child->write(dst);
}

StackWrapperCornerMenu::StackWrapperCornerMenu() {
    setFixedHeight(MIN_WIDGET_DIM);
    setStyleSheet("QMenuBar { border-bottom: 1px solid black; }");
    mSplitV = addAction("split v");
    mSplitH = addAction("split h");
    mClose = addAction("x");
}

void StackWrapperCornerMenu::setTarget(StackWidgetWrapper * const target) {
    if(mTarget) {
        disconnect(mSplitV, nullptr, mTarget, nullptr);
        disconnect(mSplitH, nullptr, mTarget, nullptr);
        disconnect(mClose, nullptr, mTarget, nullptr);
    }
    mTarget = target;
    if(mTarget) {
        connect(mSplitV, &QAction::triggered,
                mTarget, &StackWidgetWrapper::splitV);
        connect(mSplitH, &QAction::triggered,
                mTarget, &StackWidgetWrapper::splitH);
        connect(mClose, &QAction::triggered,
                mTarget, &StackWidgetWrapper::closeWrapper);
    }
}
