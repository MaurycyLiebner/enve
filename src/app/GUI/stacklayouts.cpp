#include "stacklayouts.h"

template<class SplitItemClass>
SplitItemClass* SplittableStackItem::split(WidgetPtr &&other) {
    Q_ASSERT(mParent);
    auto vStack = std::make_unique<SplitItemClass>();
    const auto vStackPtr = vStack.get();
    UniPtr thisUni = mParent->replaceChild(this, std::move(vStack));
    vStackPtr->setChildren(std::move(thisUni), std::move(other));
    return vStackPtr;
}

VSplitStackItem* SplittableStackItem::splitV(WidgetPtr &&other) {
    return split<VSplitStackItem>(std::move(other));
}

HSplitStackItem* SplittableStackItem::splitH(WidgetPtr &&other) {
    return split<HSplitStackItem>(std::move(other));
}

void ParentStackLayoutItem::sWriteChild(
        StackLayoutItem * const child, QIODevice * const dst) {
    child->writeType(dst);
    child->write(dst);
}
