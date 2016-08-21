#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H
#include "connectedtomainwindow.h"
#include <QPointF>

class Bone;

class Transformable : public ConnectedToMainWindow
{
public:
    Transformable(MainWindow *mainWindow);
    Transformable(ConnectedToMainWindow *parent);

    virtual void startTransform() {}
    virtual void finishTransform() {}

    virtual void moveBy(QPointF absTranslatione) { Q_UNUSED(absTranslatione); }

    virtual void scale(qreal scaleXBy, qreal scaleYBy) {
        Q_UNUSED(scaleXBy); Q_UNUSED(scaleYBy);
    }
    virtual void scale(qreal scaleBy) {
        scale(scaleBy, scaleBy);
    }

    virtual void rotateBy(qreal rot) {
        Q_UNUSED(rot);
    }

    virtual QPointF getAbsBoneAttachPoint() { return QPointF(); }

    bool isSelected() { return mSelected; }

    virtual bool isBeingTransformed() { return mSelected; }

    virtual void saveTransformPivot(QPointF absPivot) { Q_UNUSED(absPivot); }

    void setBone(Bone *bone, bool saveUndoRedo = true);

    virtual void cancelTransform() {}

    virtual void attachToBoneFromSqlZId() {}
protected:
    QPointF mSavedTransformPivot;
    bool mSelected = false;
    Bone *mBone = NULL;
    int mSqlLoadBoneZId = -1;
};

#endif // TRANSFORMABLE_H
