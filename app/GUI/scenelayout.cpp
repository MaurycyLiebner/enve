#include "scenelayout.h"
#include "document.h"

SceneLayout::SceneLayout(Document& document,
                         AudioHandler& audioHandler,
                         QWidget * const parent) : QWidget(parent),
    mDocument(document), mAudioHandler(audioHandler) {
    setLayout(new QHBoxLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    reset();
}

SceneBaseStackItem::cUPtr SceneLayout::extract() {
    return std::move(mBaseStack);
}

void SceneLayout::reset(CanvasWindowWrapper** const cwwP) {
    mBaseStack = std::make_unique<CWSceneBaseStackItem>();
    const auto cwwItem = static_cast<CWWidgetStackLayoutItem*>(
                mBaseStack->getChild());
    const auto cww = new CanvasWindowWrapper(&mDocument, &mAudioHandler,
                                             cwwItem);
    cww->disableClose();
    if(cwwP) *cwwP = cww;
    else setWidget(cww);
}

void SceneLayout::setCurrent(const SceneBaseStackItem* const item) {
    CanvasWindowWrapper* cwwP = nullptr;
    if(!item) return reset();
    reset(&cwwP);
    item->apply(cwwP);
    QWidget* mainW = cwwP;
    while(mainW->parentWidget())
        mainW = mainW->parentWidget();
    setWidget(mainW);
    mBaseStack->setScene(item->getScene());
}

void SceneLayout::setWidget(QWidget * const wid) {
    while(layout()->count() > 0) {
        const auto item = layout()->takeAt(0);
        delete item->widget();
        delete item->layout();
        delete item;
    }
    layout()->addWidget(wid);
}
