#include "scenelayout.h"
#include "document.h"

SceneLayout::SceneLayout(Document& document,
                         QMainWindow* const window) : QObject(window),
    mDocument(document), mWindow(window) {
    reset();
    connect(&mDocument, &Document::sceneCreated,
            this, &SceneLayout::newForScene);
    connect(&mDocument, qOverload<Canvas*>(&Document::sceneRemoved),
            this, &SceneLayout::removeForScene);
    connect(this, &SceneLayout::created,
            this, [this](const int id) {
        if(mCurrentId == -1) return;
        if(id <= mCurrentId) mCurrentId++;
    });
}

void saveAllChildrenLayoutsData(QWidget* const parent) {
    const auto cww = dynamic_cast<CanvasWindowWrapper*>(parent);
    if(cww) {
        cww->saveDataToLayout();
        return;
    }
    const auto children = parent->children();
    for(const auto& child : children) {
        const auto childWidget = qobject_cast<QWidget*>(child);
        if(!childWidget) continue;
        const auto cww = dynamic_cast<CanvasWindowWrapper*>(childWidget);
        if(cww) cww->saveDataToLayout();
        else saveAllChildrenLayoutsData(childWidget);
    }
}

void SceneLayout::reset(CanvasWindowWrapper** const cwwP) {
    if(mCurrentId != -1) {
        saveAllChildrenLayoutsData(mWindow->centralWidget());
        mCollection.replaceCustomLayout(mCurrentId, std::move(mBaseStack));
    }
    mCurrentId = -1;

    auto cwwItem = new CWWidgetStackLayoutItem;
    mBaseStack = std::make_unique<SceneBaseStackItem>(cwwItem);
    const auto cww = new CanvasWindowWrapper(&mDocument, cwwItem);
    //cww->setScene(scene);
    cww->disableClose();
    if(cwwP) *cwwP = cww;
    else mWindow->setCentralWidget(cww);
}

void SceneLayout::setCurrent(const int id) {
    CanvasWindowWrapper* cwwP = nullptr;
    reset(&cwwP);
    const auto stack = mCollection.getAt(id);
    if(!stack) {
        mCurrentId = -1;
        return;
    }
    stack->apply(cwwP);
    QWidget* mainW = cwwP;
    while(mainW->parentWidget())
        mainW = mainW->parentWidget();
    mWindow->setCentralWidget(mainW);
    mCurrentId = id;
    mBaseStack->setName(stack->getName());
    if(!mCollection.isCustom(id)) {
        static_cast<SceneBaseStackItem*>(mBaseStack.get())->setScene(
                    static_cast<const SceneBaseStackItem*>(stack)->getScene());
    }
    emit currentSet(mCurrentId);
}

void SceneLayout::removeCurrent() {
    remove(mCurrentId);
}

void SceneLayout::remove(const int id) {
    if(id == -1) return;
    if(mCollection.isCustom(id)) {
        if(mCollection.removeCustomLayout(id)) {
            if(id == mCurrentId) mCurrentId = -1;
            emit removed(id);
        }
    } else {
        if(mCollection.resetSceneLayout(id)) {
            if(id == mCurrentId) setCurrent(id);
        }
    }
}

QString SceneLayout::duplicate() {
    QString name;
    if(mCollection.isCustom(mCurrentId)) {
        name = mBaseStack->getName().trimmed();
        QRegExp exp("(.*)([0-9]+)$");
        if(exp.exactMatch(name)) {
            const int nameId = exp.cap(2).toInt();
            name = exp.cap(1) + QString::number(nameId + 1);
        } else name += " 0";
    } else {
        name = "Layout " + QString::number(mCollection.customCount());
    }

    mBaseStack->setName(name);
    auto dupli = std::make_unique<BaseStackItem>();
    dupli->setName(name);
    mCurrentId = mCollection.addCustomLayout(std::move(dupli));
    emit created(mCurrentId, name);
    return name;
}

QString SceneLayout::newEmpty() {
    reset();
    const QString name = "Layout " + QString::number(mCollection.customCount());
    mBaseStack->setName(name);
    auto newL = std::make_unique<BaseStackItem>();
    newL->setName(name);
    const int newId = mCollection.addCustomLayout(std::move(newL));
    emit created(newId, name);
    setCurrent(newId);
    return name;
}

void SceneLayout::newForScene(Canvas * const scene) {
    const int id = mCollection.addSceneLayout(scene);
    connect(scene, &Canvas::nameChanged,
            this, [this, scene]() { sceneNameChanged(scene); });
    emit created(id, scene->getName());
}

void SceneLayout::removeForScene(Canvas * const scene) {
    const int id = mCollection.removeSceneLayout(scene);
    if(mCurrentId == id) mCurrentId = -1;
    emit removed(id);
}

void SceneLayout::setCurrentName(const QString &name) {
    if(mCurrentId >= mCollection.customCount()) return;
    setName(mCurrentId, name);
}

void SceneLayout::sceneNameChanged(Canvas * const scene) {
    const int id = mCollection.idForScene(scene);
    setName(id, scene->getName());
}

void SceneLayout::setName(const int id, const QString &name) {
    mCollection.getAt(id)->setName(name);
    emit renamed(id, name);
}
