#include "canvaswindowwrapper.h"

#include <QComboBox>
#include <QLabel>

#include "GUI/newcanvasdialog.h"
#include "canvaswindow.h"
#include "document.h"
#include "scenechooser.h"
#include "audiohandler.h"

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
                                         AudioHandler * const audioHandler,
                                         CWWidgetStackLayoutItem* const layItem,
                                         QWidget * const parent) :
    StackWidgetWrapper(
        layItem,
        [document, audioHandler]() {
            const auto rPtr = new CWWidgetStackLayoutItem(*document, *audioHandler);
            return std::unique_ptr<WidgetStackLayoutItem>(rPtr);
        },
        [document, audioHandler](WidgetStackLayoutItem* const layItem,
                                 QWidget * const parent) {
            const auto cLayItem = static_cast<CWWidgetStackLayoutItem*>(layItem);
            const auto derived = new CanvasWindowWrapper(document, audioHandler,
                                                         cLayItem, parent);
            return static_cast<StackWidgetWrapper*>(derived);
        },
        [document, audioHandler](StackWidgetWrapper * toSetup) {
            const auto window = new CanvasWindow(*document, *audioHandler,
                                                 toSetup);
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
    const auto sceneWidget = getSceneWidget();
    lItem->setTransform(sceneWidget->getViewTransform());
    lItem->setScene(sceneWidget->getCurrentCanvas());
}

CanvasWindow* CanvasWindowWrapper::getSceneWidget() const {
    return static_cast<CanvasWindow*>(getCentralWidget());
}

void CWWidgetStackLayoutItem::clear() {
    SceneWidgetStackLayoutItem::clear();
    mTransform.reset();
}

QWidget* CWWidgetStackLayoutItem::create(QWidget* const parent) {
    const auto cwWrapper = new CanvasWindowWrapper(
                &mDocument, &mAudioHandler, this, parent);
    cwWrapper->setScene(mScene);
    const auto cw = cwWrapper->getSceneWidget();
    cw->blockAutomaticSizeFit();
    cw->setViewTransform(mTransform);
    return cwWrapper;
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
