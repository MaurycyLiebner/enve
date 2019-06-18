#include "canvaswindowwrapper.h"

#include <QComboBox>

#include "canvaswindow.h"
#include "document.h"

class CanvasWrapperMenuBar : public StackWrapperMenu {
public:
    CanvasWrapperMenuBar(Document& document, CanvasWindow * const window) :
        mDocument(document), mWindow(window) {
        mSceneMenu = addMenu("none");
        mSceneMenu->setDisabled(true);
        for(const auto& scene : mDocument.fScenes)
            addScene(scene.get());

        connect(&mDocument, qOverload<Canvas*>(&Document::sceneRemoved),
                this, &CanvasWrapperMenuBar::removeScene);
        connect(&mDocument, &Document::sceneCreated,
                this, &CanvasWrapperMenuBar::addScene);
    }
private:
    void addScene(Canvas * const scene) {
        if(!scene) return;
        if(mSceneToAct.empty()) mSceneMenu->setEnabled(true);
        const auto act = mSceneMenu->addAction(scene->getName());
        act->setCheckable(true);
        connect(act, &QAction::triggered, this,
                [this, scene, act]() { setCurrentScene(scene, act); });
        connect(scene, &Canvas::canvasNameChanged, act,
                [this, act](Canvas* const scene, const QString& name) {
            if(scene == mCurrentScene) mSceneMenu->setTitle(name);
            act->setText(name);
        });
        mSceneToAct.insert({scene, act});
        if(!mCurrentScene) setCurrentScene(scene, act);
    }

    void removeScene(Canvas * const scene) {
        const auto removeIt = mSceneToAct.find(scene);
        if(removeIt == mSceneToAct.end()) return;
        if(mCurrentScene == scene) {
            const auto newIt = mSceneToAct.begin();
            if(newIt == mSceneToAct.end()) setCurrentScene(nullptr, nullptr);
            else {
                setCurrentScene(newIt->first, newIt->second);
            }
        }
        mSceneMenu->removeAction(removeIt->second);
        delete removeIt->second;
        mSceneToAct.erase(removeIt);
        if(mSceneToAct.empty()) mSceneMenu->setDisabled(true);
    }

    void setCurrentScene(Canvas * const scene, QAction * const act) {
        if(act) {
            act->setChecked(true);
            act->setDisabled(true);
        } if(mCurrentScene) {
            const auto currAct = mSceneToAct[mCurrentScene];
            currAct->setChecked(false);
            currAct->setEnabled(true);
        }
        mSceneMenu->setTitle(scene ? scene->getName() : "none");
        mWindow->setCurrentCanvas(scene);
        mCurrentScene = scene;
    }

    Document& mDocument;
    CanvasWindow * const mWindow;
    QMenu * mSceneMenu;
    Canvas * mCurrentScene = nullptr;
    std::map<Canvas*, QAction*> mSceneToAct;
};
#include <QLabel>
CanvasWindowWrapper::CanvasWindowWrapper(Document * const document,
                                         QWidget * const parent) :
    StackWidgetWrapper(
        [document](StackWidgetWrapper * toSetup) {
            const auto window = new CanvasWindow(*document, toSetup);
            toSetup->setMenuBar(new CanvasWrapperMenuBar(*document, window));
            toSetup->setCentralWidget(window->getCanvasWidget());
        }, parent) {}
