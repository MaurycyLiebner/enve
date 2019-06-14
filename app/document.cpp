#include "document.h"
#include "FileCacheHandlers/filecachehandler.h"
#include "canvas.h"

bool Document::FileCompare::operator()(const FileHandler &f1,
                                       const FileHandler &f2) {
    return f1->getFilePath() < f2->getFilePath();
}

Canvas *Document::createNewCanvas() {
    const auto newCanvas = SPtrCreate(Canvas)();
    fScenes.append(newCanvas);
    return newCanvas.get();
}

void Document::write(QIODevice * const dst) const {

}

void Document::read(QIODevice * const src) const {

}

void Document::SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                                    const UpdateFuncs &updateFuncs,
                                    const int visiblePartWidgetId) {
    for(const auto& scene : fScenes) {
        auto abs = scene->SWT_abstractionForWidget(updateFuncs,
                                                   visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}
