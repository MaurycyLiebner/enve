#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "layoutcollection.h"

class SceneLayout : public QWidget {
public:
    SceneLayout(Document &document, AudioHandler &audioHandler,
                QWidget* const parent = nullptr);

    SceneBaseStackItem *extract();
    void setCurrent(SceneBaseStackItem * const item);
    void saveData();
private:
    void clearWidget();
    void reset();

    Document& mDocument;
    AudioHandler& mAudioHandler;

    SceneBaseStackItem* mBaseStack = nullptr;
};

#endif // SCENELAYOUT_H
