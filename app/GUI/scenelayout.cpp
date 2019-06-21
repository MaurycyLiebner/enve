#include "scenelayout.h"

SceneLayout::SceneLayout(Document& document,
                         QMainWindow* const window) :
    mDocument(document), mWindow(window),
    mBaseStack(std::make_unique<BaseStackItem>()) {
    reset();
}

void SceneLayout::reset() {
    auto cwwItem = new CWWidgetStackLayoutItem;
    mBaseStack->setChild(std::unique_ptr<WidgetStackLayoutItem>(cwwItem));
    const auto cww = new CanvasWindowWrapper(&mDocument, cwwItem, mWindow);
    cww->disableClose();
    mWindow->setCentralWidget(cww);
}
