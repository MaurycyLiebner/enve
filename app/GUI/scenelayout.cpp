#include "scenelayout.h"

SceneLayout::SceneLayout(Document& document,
                         QMainWindow* const window) :
    mDocument(document), mWindow(window) {
    reset();
}

BaseStackItem::UPtr SceneLayout::reset(CanvasWindowWrapper** const cwwP) {
    auto tmp = std::move(mBaseStack);
    mBaseStack = std::make_unique<BaseStackItem>();
    auto cwwItem = new CWWidgetStackLayoutItem;
    mBaseStack->setChild(std::unique_ptr<WidgetStackLayoutItem>(cwwItem));
    const auto cww = new CanvasWindowWrapper(&mDocument, cwwItem, mWindow);
    cww->disableClose();
    mWindow->setCentralWidget(cww);
    if(cwwP) *cwwP = cww;
    return tmp;
}

BaseStackItem::UPtr SceneLayout::apply(const BaseStackItem::UPtr& stack) {
    CanvasWindowWrapper* cwwP = nullptr;
    auto tmp = reset(&cwwP);
    stack->apply(cwwP);
    return tmp;
}
