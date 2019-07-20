#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "layoutcollection.h"

class SceneLayout : public QWidget {
public:
    SceneLayout(Document &document,
                QWidget* const parent = nullptr);

    SceneBaseStackItem *extract();
    void setCurrent(SceneBaseStackItem * const item);
    void saveData();
private:
    void clearWidget();
    void reset();

    Document& mDocument;

    SceneBaseStackItem* mBaseStack = nullptr;
};

#endif // SCENELAYOUT_H
