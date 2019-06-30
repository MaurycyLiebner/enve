#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "layoutcollection.h"

class SceneLayout : public QWidget {
public:
    SceneLayout(QWidget* const parent = nullptr);

    SceneBaseStackItem *extract();
    void setCurrent(SceneBaseStackItem * const item);
    void saveData();
private:
    void clearWidget();
    void reset();

    SceneBaseStackItem* mBaseStack = nullptr;
};

#endif // SCENELAYOUT_H
