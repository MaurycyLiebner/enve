#include "canvaswindowwrapper.h"

#include <QComboBox>

#include "canvaswindow.h"
#include "document.h"

class CanvasWrapperMenuBar : public StackWrapperMenu {
public:
    CanvasWrapperMenuBar(Document& document, CanvasWindow * const window) :
        mDocument(document), mWindow(window) {
        mLayout = new QHBoxLayout(this);
        setLayout(mLayout);
        mSceneCombo = new QComboBox(this);
        mSceneCombo->addItem("");
        mLayout->addWidget(mSceneCombo);
        for(const auto& scene : mDocument.fScenes)
            addScene(scene.get());

        connect(mSceneCombo, qOverload<int>(&QComboBox::currentIndexChanged),
                this, [this](const int id) { setCurrentSceneId(id - 1); });
        connect(&mDocument, qOverload<int>(&Document::sceneRemoved),
                this, &CanvasWrapperMenuBar::removeScene);
        connect(&mDocument, &Document::sceneCreated,
                this, &CanvasWrapperMenuBar::addScene);
    }
private:
    void addScene(Canvas * const scene) {
        mSceneCombo->addItem(scene->getName());
    }

    void removeScene(const int id) {
        mSceneCombo->removeItem(id + 1);
    }

    void setCurrentSceneId(const int id) {
        if(id < 0) setCurrentScene(nullptr);
        else setCurrentScene(mDocument.fScenes.at(id).get());
    }

    void setCurrentScene(Canvas * const scene) {
        if(!scene) mSceneCombo->setCurrentIndex(0);
        mWindow->setCurrentCanvas(scene);
    }

    Document& mDocument;
    CanvasWindow * const mWindow;
    QHBoxLayout * mLayout;
    QComboBox* mSceneCombo;
};

CanvasWindowWrapper::CanvasWindowWrapper(Document * const document,
                                         QWidget * const parent) :
    StackWidgetWrapper(
        [document](StackWidgetWrapper * toSetup) {
            const auto window = new CanvasWindow(*document, toSetup);
            toSetup->setMenuBar(new CanvasWrapperMenuBar(*document, window));
            toSetup->setCentralWidget(window->getCanvasWidget());
        }, parent) {}
