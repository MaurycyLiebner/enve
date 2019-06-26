#include "timelinelayout.h"
#include "document.h"
#include "boxeslistkeysviewwidget.h"
#include "boxeslistanimationdockwidget.h"
#include "timelinewrapper.h"

TimelineLayout::TimelineLayout(Document& document, ChangeWidthWidget * const chww,
                               BoxesListAnimationDockWidget * const window) : QObject(window),
    mCollection(LayoutCollection::sCreator<TSceneBaseStackItem>()),
    mDocument(document), mWindow(window), mChww(chww) {
    connect(&mDocument, &Document::sceneCreated,
            this, &TimelineLayout::newForScene);
    connect(&mDocument, qOverload<Canvas*>(&Document::sceneRemoved),
            this, &TimelineLayout::removeForScene);
    connect(this, &TimelineLayout::created,
            this, [this](const int id) {
        if(mCurrentId == -1) return;
        if(id <= mCurrentId) mCurrentId++;
    });
    reset();
}

void TSaveAllChildrenLayoutsData(QWidget* const parent) {
    const auto cww = dynamic_cast<TimelineWrapper*>(parent);
    if(cww) {
        cww->saveDataToLayout();
        return;
    }
    const auto children = parent->children();
    for(const auto& child : children) {
        const auto childWidget = qobject_cast<QWidget*>(child);
        if(!childWidget) continue;
        const auto cww = dynamic_cast<TimelineWrapper*>(childWidget);
        if(cww) cww->saveDataToLayout();
        else TSaveAllChildrenLayoutsData(childWidget);
    }
}

void TimelineLayout::reset(TimelineWrapper** const cwwP) {
    if(mCurrentId != -1) {
        TSaveAllChildrenLayoutsData(mWindow->centralWidget());
        mCollection.replaceCustomLayout(mCurrentId, std::move(mBaseStack));
    }
    mCurrentId = -1;

    mBaseStack = std::make_unique<TSceneBaseStackItem>();
    const auto cwwItem = static_cast<TWidgetStackLayoutItem*>(
                mBaseStack->getChild());
    const auto cww = new TimelineWrapper(&mDocument, mChww, cwwItem);
    cww->disableClose();
    if(cwwP) *cwwP = cww;
    else mWindow->setCentralWidget(cww);
}

void TimelineLayout::setCurrent(const int id) {
    TimelineWrapper* cwwP = nullptr;
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

void TimelineLayout::removeCurrent() {
    remove(mCurrentId);
}

void TimelineLayout::remove(const int id) {
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

QString TimelineLayout::duplicate() {
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

QString TimelineLayout::newEmpty() {
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

void TimelineLayout::newForScene(Canvas * const scene) {
    const int id = mCollection.addSceneLayout(scene);
    connect(scene, &Canvas::nameChanged,
            this, [this, scene]() { sceneNameChanged(scene); });
    emit created(id, scene->getName());
}

void TimelineLayout::removeForScene(Canvas * const scene) {
    const int id = mCollection.removeSceneLayout(scene);
    if(mCurrentId == id) mCurrentId = -1;
    emit removed(id);
}

void TimelineLayout::setCurrentName(const QString &name) {
    if(mCurrentId >= mCollection.customCount()) return;
    setName(mCurrentId, name);
}

void TimelineLayout::read(QIODevice * const src) {
    int nCustom;
    src->read(rcChar(&nCustom), sizeof(int));
    for(int i = 0; i < nCustom; i++) {
        auto newL = BaseStackItem::sRead<TWidgetStackLayoutItem>(src);
        const QString& name = newL->getName();
        const int id = mCollection.addCustomLayout(std::move(newL));
        emit created(id, name);
    }
}

void TimelineLayout::sceneNameChanged(Canvas * const scene) {
    const int id = mCollection.idForScene(scene);
    setName(id, scene->getName());
}

void TimelineLayout::setName(const int id, const QString &name) {
    mCollection.getAt(id)->setName(name);
    emit renamed(id, name);
}
