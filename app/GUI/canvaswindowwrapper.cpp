#include "canvaswindowwrapper.h"

#include <QComboBox>

#include "canvaswindow.h"
#include "document.h"

class CanvasWrapperMenuBar : public StackWrapperMenu {
public:
    CanvasWrapperMenuBar(Document& document, CanvasWindow * const window) :
        mDocument(document), mWindow(window) {
        mSceneCombo = new QComboBox(this);
        mSceneCombo->setMinimumContentsLength(10);
        mSceneCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        for(const auto& scene : mDocument.fScenes)
            addScene(scene.get());
        addWidget(mSceneCombo);

        connect(mSceneCombo, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &CanvasWrapperMenuBar::setCurrentSceneId);
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
        mSceneCombo->removeItem(id);
    }

    void setCurrentSceneId(const int id) {
        if(id < 0) mWindow->setCurrentCanvas(nullptr);
        else mWindow->setCurrentCanvas(mDocument.fScenes.at(id).get());
    }

    Document& mDocument;
    CanvasWindow * const mWindow;
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
