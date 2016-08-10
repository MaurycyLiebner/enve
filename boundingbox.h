#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "connectedtomainwindow.h"

class UndoRedo;

class Canvas;

class UndoRedoStack;

class BoundingBox : public ConnectedToMainWindow
{
public:
    BoundingBox(BoundingBox *parent);
    BoundingBox(MainWindow *window);

    QMatrix getCombinedTransform();

    QPointF getTranslation();

    void scale(qreal scaleBy, QPointF absOrigin);
    void scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin);

    void rotateBy(qreal rot, QPointF absOrigin);
    void moveBy(qreal dx, qreal dy);

    void setTransformation(QMatrix transMatrix);

    QPointF getAbsolutePos();

    void updateCombinedTransform();
    void moveBy(QPointF trans);

    void startTransform();
    void finishTransform();

    void addChild(BoundingBox *child);
    void removeChild(BoundingBox *child);
protected:
    virtual void updateAfterCombinedTransformationChanged();

    QList<BoundingBox*> mChildren;
    BoundingBox *mParent = NULL;
    QMatrix mSavedTransformMatrix;
    QMatrix mTransformMatrix;
    QMatrix mCombinedTransformMatrix;
};


#endif // CHILDPARENT_H
