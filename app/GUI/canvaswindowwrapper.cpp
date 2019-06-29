#include "canvaswindowwrapper.h"

#include <QComboBox>
#include <QLabel>

#include "GUI/newcanvasdialog.h"
#include "canvaswindow.h"
#include "document.h"
#include "scenechooser.h"

class CanvasWrapperMenuBar : public StackWrapperMenu {
    friend struct CWWidgetStackLayoutItem;
    friend class CanvasWindowWrapper;
public:
    CanvasWrapperMenuBar(Document& document, CanvasWindow * const window) :
        mDocument(document), mWindow(window) {
        mSceneMenu = new SceneChooser(mDocument, false, this);
        addMenu(mSceneMenu);
        connect(mSceneMenu, &SceneChooser::currentChanged,
                this, &CanvasWrapperMenuBar::setCurrentScene);
    }
private:
    void setCurrentScene(Canvas * const scene) {
        mWindow->setCurrentCanvas(scene);
        mSceneMenu->setCurrentScene(scene);
    }

    Canvas* getCurrentScene() const { return mCurrentScene; }

    Document& mDocument;
    CanvasWindow* const mWindow;
    SceneChooser * mSceneMenu;
    Canvas * mCurrentScene = nullptr;
    std::map<Canvas*, QAction*> mSceneToAct;
};

CanvasWindowWrapper::CanvasWindowWrapper(Document * const document,
                                         CWWidgetStackLayoutItem* const layItem,
                                         QWidget * const parent) :
    StackWidgetWrapper(
        layItem,
        []() {
            const auto rPtr = new CWWidgetStackLayoutItem;
            return std::unique_ptr<WidgetStackLayoutItem>(rPtr);
        },
        [document](WidgetStackLayoutItem* const layItem,
                   QWidget * const parent) {
            const auto cLayItem = static_cast<CWWidgetStackLayoutItem*>(layItem);
            const auto derived = new CanvasWindowWrapper(document, cLayItem, parent);
            return static_cast<StackWidgetWrapper*>(derived);
        },
        [document](StackWidgetWrapper * toSetup) {
            const auto window = new CanvasWindow(*document, toSetup);
            toSetup->setCentralWidget(window);
            toSetup->setMenuBar(new CanvasWrapperMenuBar(*document, window));
}, parent) {}

void CanvasWindowWrapper::setScene(Canvas * const scene) {
    const auto menu = static_cast<CanvasWrapperMenuBar*>(getMenuBar());
    menu->setCurrentScene(scene);
}

Canvas* CanvasWindowWrapper::getScene() const {
    const auto menu = static_cast<CanvasWrapperMenuBar*>(getMenuBar());
    return menu->getCurrentScene();
}

void CanvasWindowWrapper::saveDataToLayout() const {
    const auto lItem = static_cast<CWWidgetStackLayoutItem*>(getLayoutItem());
    if(!lItem) return;
    const auto sceneWidget = getSceneWidget();
    lItem->setTransform(sceneWidget->getViewTransform());
    lItem->setScene(sceneWidget->getCurrentCanvas());
}

CanvasWindow* CanvasWindowWrapper::getSceneWidget() const {
    return static_cast<CanvasWindow*>(getCentralWidget());
}

void CanvasWindowWrapper::changeEvent(QEvent *e) {
    StackWidgetWrapper::changeEvent(e);
    if(e->type() == QEvent::ParentChange) {
        const auto sceneWidget = getSceneWidget();
        if(sceneWidget && !sceneWidget->hasNoCanvas()) {
            sceneWidget->unblockAutomaticSizeFit();
            sceneWidget->fitCanvasToSize();
        }
    }
}

void CWWidgetStackLayoutItem::clear() {
    SceneWidgetStackLayoutItem::clear();
    mTransform.reset();
}

void CWWidgetStackLayoutItem::apply(StackWidgetWrapper * const stack) const {
    const auto cwWrapper = static_cast<CanvasWindowWrapper*>(stack);
    cwWrapper->setScene(mScene);
    const auto cw = cwWrapper->getSceneWidget();
    cw->blockAutomaticSizeFit();
    cw->setViewTransform(mTransform);
}

void CWWidgetStackLayoutItem::write(QIODevice * const dst) const {
    SceneWidgetStackLayoutItem::write(dst);
    dst->write(rcConstChar(&mTransform), sizeof(QMatrix));
}

void CWWidgetStackLayoutItem::read(QIODevice * const src) {
    SceneWidgetStackLayoutItem::read(src);
    src->read(rcChar(&mTransform), sizeof(QMatrix));
}

void CWWidgetStackLayoutItem::setTransform(const QMatrix& transform) {
    mTransform = transform;
}

void SceneWidgetStackLayoutItem::clear() {
    setScene(nullptr);
}

void SceneWidgetStackLayoutItem::write(QIODevice * const dst) const {
    const int sceneId = mScene ? mScene->getWriteId() : -1;
    dst->write(rcConstChar(&sceneId), sizeof(int));
}

void SceneWidgetStackLayoutItem::read(QIODevice * const src) {
    int sceneId;
    src->read(rcChar(&sceneId), sizeof(int));
    const auto sceneBox = BoundingBox::sGetBoxByReadId(sceneId);
    const auto scene = dynamic_cast<Canvas*>(sceneBox);
    setScene(scene);
}

void SceneWidgetStackLayoutItem::setScene(Canvas * const scene) {
    mScene = scene;
}
