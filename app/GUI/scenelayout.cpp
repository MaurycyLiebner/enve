#include "scenelayout.h"

SceneLayout::SceneLayout(Document& document,
                         QMainWindow* const window) :
    mDocument(document), mWindow(window) {
    reset();
}

BaseStackItem::UPtr SceneLayout::reset(CanvasWindowWrapper** const cwwP) {
    if(mCurrentId != -1)
        mCollection.replaceCustomLayout(mCurrentId, std::move(mBaseStack));
    auto tmp = std::move(mBaseStack);
    mBaseStack = std::make_unique<BaseStackItem>();
    mCurrentId = -1;
    auto cwwItem = new CWWidgetStackLayoutItem;
    mBaseStack->setChild(std::unique_ptr<WidgetStackLayoutItem>(cwwItem));
    const auto cww = new CanvasWindowWrapper(&mDocument, cwwItem, mWindow);
    cww->disableClose();
    mWindow->setCentralWidget(cww);
    if(cwwP) *cwwP = cww;
    return tmp;
}

BaseStackItem::UPtr SceneLayout::apply(const int id) {
    CanvasWindowWrapper* cwwP = nullptr;
    auto tmp = reset(&cwwP);
    const auto stack = mCollection.getAt(id);
    stack->apply(cwwP);
    mCurrentId = id;
    mBaseStack->setName(stack->getName());
    return tmp;
}

void SceneLayout::duplicateLayout(const int id) {
    apply(id);
    QString name;
    if(mCollection.isCustom(id)) {
        name = mBaseStack->getName().trimmed();
        QRegExp exp("(.*)([0-9]+)$");
        if(exp.exactMatch(name)) {
            const int nameId = exp.cap(2).toInt();
            name = exp.cap(1) + QString::number(nameId + 1);
        } else name += " 0";
    } else {
        name = "Layout " + QString::number(mCollection.customCount());
    }

    auto dupli = std::make_unique<BaseStackItem>();
    dupli->setName(name);
    mCurrentId = mCollection.addCustomLayout(std::move(dupli));
}
