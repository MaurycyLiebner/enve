#ifndef BONE_H
#define BONE_H
#include "vectorpath.h"

class Bone : public VectorPath
{
public:
    Bone(BoxesGroup *parent);
    Bone(int boundingBoxId, BoxesGroup *parent);

    void moveBy(QPointF trans);
    void startTransform();
    void finishTransform();
    void rotateBy(qreal rot);
    void scale(qreal scaleBy);
    void saveTransformPivot(QPointF absPivot);
    void addTransformable(Transformable *transformable);
    void removeTransformable(Transformable *transformable);
    void addCircle(QPointF absPos);
    void cancelTransform();
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);
private:
    QList<Transformable*> mTransformables;
};

#endif // BONE_H
