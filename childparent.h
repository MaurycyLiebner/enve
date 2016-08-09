#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "connectedtomainwindow.h"

class UndoRedo;

class Canvas;

class UndoRedoStack;

class ChildParent : public ConnectedToMainWindow
{
public:
    ChildParent(Canvas *canvas);

    QMatrix getCombinedTransform();

    void setParent(ChildParent *parent);
    QPointF getTranslation();
    void rotateBy(qreal rot, QPointF absOrigin);
    void moveBy(qreal dx, qreal dy);

    void setTransformation(QMatrix transMatrix);

    QPointF getAbsolutePos();

    void updateCombinedTransform();
    void moveBy(QPointF trans);

    void startTransform();
    void finishTransform();
protected:
    virtual void updateAfterTransformationChanged();
    virtual void updateAfterCombinedTransformationChanged();

    Canvas *mCanvas;
    QMatrix mSavedTransformMatrix;
    qreal mRot = 0.f;
    QMatrix mTransformMatrix;
    QMatrix mCombinedTransformMatrix;
    ChildParent *mParent = NULL;
    QList<ChildParent*> mChildren;
};

#endif // CHILDPARENT_H
