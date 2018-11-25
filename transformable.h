#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H
#include <QPointF>
#include "skiaincludes.h"

class Transformable {
public:
    Transformable();
    virtual ~Transformable() {}


    virtual void moveByRel(const QPointF &relTranslatione) {
        Q_UNUSED(relTranslatione);
    }
    virtual void moveByAbs(const QPointF &absTranslatione) {
        Q_UNUSED(absTranslatione);
    }

    virtual void scale(const qreal &scaleXBy,
                       const qreal &scaleYBy) {
        Q_UNUSED(scaleXBy); Q_UNUSED(scaleYBy);
    }



    virtual void saveTransformPivotAbsPos(const QPointF& absPivot) {
        Q_UNUSED(absPivot);
    }

    virtual void cancelTransform() {}
protected:

};

#endif // TRANSFORMABLE_H
