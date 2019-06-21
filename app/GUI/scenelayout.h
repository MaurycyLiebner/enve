#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "canvaswindowwrapper.h"

class SceneLayout {
public:
    SceneLayout(Document& document, QMainWindow* const window);

    void reset();
private:

    Document& mDocument;
    QMainWindow* const mWindow;
    std::unique_ptr<BaseStackItem> mBaseStack;
    //std::unique_ptr<Ca
};

#endif // SCENELAYOUT_H
