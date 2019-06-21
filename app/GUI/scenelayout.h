#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "canvaswindowwrapper.h"

class SceneLayout {
public:
    SceneLayout(Document& document, QMainWindow* const window);

    BaseStackItem::UPtr reset(CanvasWindowWrapper ** const cwwP = nullptr);
    BaseStackItem::UPtr apply(const BaseStackItem::UPtr &stack);
private:
    Document& mDocument;
    QMainWindow* const mWindow;
    BaseStackItem::UPtr mBaseStack;
};

#endif // SCENELAYOUT_H
