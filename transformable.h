#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H
#include <QPointF>

class Transformable
{
public:
    Transformable();

    virtual void startTransform() {}
    virtual void finishTransform() {}

    virtual void moveByRel(const QPointF &relTranslatione) {
        Q_UNUSED(relTranslatione);
    }
    virtual void moveByAbs(const QPointF &absTranslatione) {
        Q_UNUSED(absTranslatione);
    }

    virtual void scale(const qreal &scaleXBy, const qreal &scaleYBy) {
        Q_UNUSED(scaleXBy); Q_UNUSED(scaleYBy);
    }
    virtual void scale(const qreal &scaleBy) {
        scale(scaleBy, scaleBy);
    }

    virtual void rotateBy(const qreal &rot) {
        Q_UNUSED(rot);
    }

    bool isSelected() { return mSelected; }

    virtual bool isBeingTransformed() { return mSelected; }

    virtual void saveTransformPivotAbsPos(QPointF absPivot) {
        Q_UNUSED(absPivot);
    }

    virtual void cancelTransform() {}
protected:
    QPointF mSavedTransformPivot;
    bool mSelected = false;
};

#endif // TRANSFORMABLE_H
