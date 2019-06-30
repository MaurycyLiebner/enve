#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "layoutcollection.h"

class SceneLayout : public QWidget {
public:
    SceneLayout(QWidget* const parent = nullptr);

    SceneBaseStackItem *extract();
    void setCurrent(SceneBaseStackItem * const item);

//    void write(QIODevice* const dst) const {
//        const int nCustom = mCollection.customCount();
//        dst->write(rcConstChar(&nCustom), sizeof(int));
//        for(int i = 0; i < nCustom; i++) {
//            if(i == mCurrentId) mBaseStack->write(dst);
//            else mCollection.getAt(i)->write(dst);
//        }
//    }

//    void read(QIODevice* const src) {
//        int nCustom;
//        src->read(rcChar(&nCustom), sizeof(int));
//        for(int i = 0; i < nCustom; i++) {
//            auto newL = BaseStackItem::sRead<CWWidgetStackLayoutItem>(src);
//            const QString& name = newL->getName();
//            const int id = mCollection.addCustomLayout(std::move(newL));
//        }
//    }
private:
    void setWidget(QWidget * const wid);
    void reset();

    SceneBaseStackItem* mBaseStack = nullptr;
};

#endif // SCENELAYOUT_H
