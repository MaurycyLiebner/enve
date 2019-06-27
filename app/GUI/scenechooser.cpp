#include "scenechooser.h"
#include "document.h"
#include "canvas.h"

SceneChooser::SceneChooser(Document& document, const bool active,
                           QWidget* const parent) :
    QMenu("none", parent), mDocument(document) {
    if(active) {
        const auto active = addAction("Active");
        active->setCheckable(true);
        addSeparator();
        connect(active, &QAction::toggled,
                this, [this](const bool active) {
            if(active) {
                connect(&mDocument, &Document::activeSceneSet,
                        this, qOverload<Canvas*>(&SceneChooser::setCurrentScene));
                setCurrentScene(mDocument.fActiveScene);
            } else {
                disconnect(&mDocument, &Document::activeSceneSet,
                           this, qOverload<Canvas*>(&SceneChooser::setCurrentScene));
            }
        });
    }
    for(const auto& scene : mDocument.fScenes)
        addScene(scene.get());

    connect(&mDocument, qOverload<Canvas*>(&Document::sceneRemoved),
            this, &SceneChooser::removeScene);
    connect(&mDocument, &Document::sceneCreated,
            this, &SceneChooser::addScene);

    if(isEmpty()) setDisabled(true);
}

void SceneChooser::addScene(Canvas * const scene) {
    if(!scene) return;
    if(isEmpty()) setEnabled(true);
    const auto act = addAction(scene->getName());
    act->setCheckable(true);
    connect(act, &QAction::triggered, this,
            [this, scene, act]() { setCurrentScene(scene, act); });
    connect(scene, &Canvas::canvasNameChanged, act,
            [this, act](Canvas* const scene, const QString& name) {
        if(scene == mCurrentScene) setTitle(name);
        act->setText(name);
    });
    mSceneToAct.insert({scene, act});
    //if(!mCurrentScene) setCurrentScene(scene, act);
}

void SceneChooser::removeScene(Canvas * const scene) {
    const auto removeIt = mSceneToAct.find(scene);
    if(removeIt == mSceneToAct.end()) return;
    if(mCurrentScene == scene) {
        const auto newIt = mSceneToAct.begin();
        if(newIt == mSceneToAct.end()) setCurrentScene(nullptr);
        else setCurrentScene(newIt->first, newIt->second);
    }
    removeAction(removeIt->second);
    delete removeIt->second;
    mSceneToAct.erase(removeIt);
    if(isEmpty()) setDisabled(true);
}

void SceneChooser::setCurrentScene(Canvas * const scene) {
    if(scene == mCurrentScene) return;
    if(!scene) return setCurrentScene(nullptr, nullptr);
    const auto it = mSceneToAct.find(scene);
    if(it == mSceneToAct.end()) return;
    setCurrentScene(scene, it->second);
}

void SceneChooser::setCurrentScene(Canvas * const scene, QAction * const act) {
    if(act) {
        act->setChecked(true);
        act->setDisabled(true);
    }
    if(mCurrentScene) {
        const auto currAct = mSceneToAct[mCurrentScene];
        currAct->setChecked(false);
        currAct->setEnabled(true);
    }
    setTitle(scene ? scene->getName() : "none");
    mCurrentScene = scene;
    emit currentChanged(mCurrentScene);
}
