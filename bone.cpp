#include "bone.h"

Bone::Bone(BoxesGroup *parent) : VectorPath(parent)
{
    mType = TYPE_BONE;
    setName("Bone");
}

Bone::Bone(int boundingBoxId, BoxesGroup *parent) :
    VectorPath(boundingBoxId, parent)
{
    mType = TYPE_BONE;
}

void Bone::addCircle(QPointF absPos) {
    qreal radius = 10.f;
    PathPoint *firstPoint = addPointAbsPos(absPos + QPointF(radius, 0.f), NULL);
    PathPoint *point = firstPoint->addPointAbsPos(absPos + QPointF(0.f, radius));
    point = point->addPointAbsPos(absPos + QPointF(-radius, 0.f));
    point = point->addPointAbsPos(absPos + QPointF(0.f, -radius));
    firstPoint->connectToPoint(point);

    mFillPaintSettings.setCurrentColor(Color(1.f, 1.f, 1.f, 0.5f) );
    mStrokeSettings.setCurrentColor(Color(1.f, 0.f, 0.f, 1.f) );
}

void Bone::cancelTransform()
{
    VectorPath::cancelTransform();
    foreach(Transformable *transformable, mTransformables) {
        transformable->cancelTransform();
    }
}

void Bone::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    QPointF centerPoint = getPivotAbsPos();
    p->setPen(QPen(Qt::black, 1.f, Qt::DotLine));
    foreach(Transformable *transformable, mTransformables) {
        p->drawLine(centerPoint, transformable->getAbsBoneAttachPoint());
    }
    VectorPath::drawSelected(p, currentCanvasMode);
}

void Bone::moveBy(QPointF trans)
{
    VectorPath::moveBy(trans);
    foreach(Transformable *transformable, mTransformables) {
        if(transformable->isBeingTransformed()) continue;
        transformable->moveBy(trans);
    }
}

void Bone::startTransformablesTransform() {
    foreach(Transformable *transformable, mTransformables) {
        if(transformable->isBeingTransformed()) {
            continue;
        }
        transformable->startTransform();
    }
}

void Bone::startTransform()
{
    VectorPath::startTransform();
    startTransformablesTransform();
}

void Bone::startRotTransform()
{
    VectorPath::startRotTransform();
    startTransformablesTransform();
}

void Bone::startPosTransform()
{
    VectorPath::startPosTransform();
    startTransformablesTransform();
}

void Bone::startScaleTransform()
{
    VectorPath::startScaleTransform();
    startTransformablesTransform();
}

void Bone::finishTransform()
{
    startNewUndoRedoSet();

    VectorPath::finishTransform();
    foreach(Transformable *transformable, mTransformables) {
        if(transformable->isBeingTransformed()) {
            continue;
        }
        transformable->finishTransform();
    }

    finishUndoRedoSet();
}

void Bone::rotateBy(qreal rot)
{
    VectorPath::rotateBy(rot);
    foreach(Transformable *transformable, mTransformables) {
        if(transformable->isBeingTransformed()) {
            continue;
        }
        transformable->rotateBy(rot);
    }
}

void Bone::scale(qreal scaleBy)
{
    VectorPath::scale(scaleBy);
    foreach(Transformable *transformable, mTransformables) {
        if(transformable->isBeingTransformed()) {
            continue;
        }
        transformable->scale(scaleBy);
    }
}

void Bone::saveTransformPivot(QPointF absPivot)
{
    VectorPath::saveTransformPivot(absPivot);
    foreach(Transformable *transformable, mTransformables) {
        if(transformable->isBeingTransformed()) {
            continue;
        }
        transformable->saveTransformPivot(absPivot);
    }
}

void Bone::addTransformable(Transformable *transformable)
{
    mTransformables << transformable;
    scheduleRepaint();
}

void Bone::removeTransformable(Transformable *transformable)
{
    mTransformables.removeOne(transformable);
    scheduleRepaint();
}
