#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "layoutcollection.h"

class SceneLayout : public QWidget {
    Q_OBJECT
public:
    SceneLayout(Document& document, QWidget* const parent);

    void setCurrent(const int id);
    void remove(const int id);
    void removeCurrent();
    QString duplicate();
    QString newEmpty();
    void setCurrentName(const QString& name);
    QStringList getCustomNames() const {
        return mCollection.getCustomNames();
    }

    QStringList getSceneNames() const {
        return mCollection.getSceneNames();
    }

    bool isCurrentCustom() const {
        return mCollection.isCustom(mCurrentId);
    }

    void write(QIODevice* const dst) const {
        const int nCustom = mCollection.customCount();
        dst->write(rcConstChar(&nCustom), sizeof(int));
        for(int i = 0; i < nCustom; i++) {
            if(i == mCurrentId) mBaseStack->write(dst);
            else mCollection.getAt(i)->write(dst);
        }
    }

    void read(QIODevice* const src) {
        int nCustom;
        src->read(rcChar(&nCustom), sizeof(int));
        for(int i = 0; i < nCustom; i++) {
            auto newL = BaseStackItem::sRead<CWWidgetStackLayoutItem>(src);
            const QString& name = newL->getName();
            const int id = mCollection.addCustomLayout(std::move(newL));
            emit created(id, name);
        }
    }

    void writeCurrentId(QIODevice* const dst) {
        dst->write(rcConstChar(&mCurrentId), sizeof(int));
    }

    void readCurrentId(QIODevice* const src) {
        int newCurrentId;
        src->read(rcChar(&newCurrentId), sizeof(int));
        setCurrent(newCurrentId);
    }

    void clear() {
        for(int i = 0; i < mCollection.customCount(); i++)
            remove(0);
    }
signals:
    void removed(int);
    void created(int, QString);
    void renamed(int, QString);
    void currentSet(int);
private:
    void setWidget(QWidget * const wid);

    void setName(const int id, const QString& name);

    void sceneNameChanged(Canvas* const scene);
    void newForScene(Canvas* const scene);
    void removeForScene(Canvas* const scene);
    void reset(CanvasWindowWrapper ** const cwwP = nullptr);

    int mCurrentId = -1;
    LayoutCollection mCollection;

    Document& mDocument;
    BaseStackItem::UPtr mBaseStack;
};

#endif // SCENELAYOUT_H
