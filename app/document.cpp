#include "document.h"
#include "FileCacheHandlers/filecachehandler.h"
#include "canvas.h"

bool Document::FileCompare::operator()(const FileHandler &f1,
                                       const FileHandler &f2) {
    return f1->getFilePath() < f2->getFilePath();
}

void Document::SWT_setupAbstraction(SingleWidgetAbstraction *abstraction,
                                           const UpdateFuncs &updateFuncs,
                                           const int visiblePartWidgetId) {
    for(const auto& scene : fScenes) {
        auto abs = scene->SWT_abstractionForWidget(updateFuncs,
                                                   visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}
