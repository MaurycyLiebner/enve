#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"
#include "canvas.h"

ExternalLinkBox::ExternalLinkBox(BoxesGroup *parent) :
    BoxesGroup(parent) {
    setType(TYPE_EXTERNAL_LINK);
    setName("Link Empty");
}

void ExternalLinkBox::reload() {
    QSqlDatabase db = QSqlDatabase::database();//not dbConnection
    db.setDatabaseName(mSrc);
    db.open();

    loadChildrenFromSql(0, false);

    db.close();

    scheduleSoftUpdate();
}

void ExternalLinkBox::changeSrc() {
    QString src = QFileDialog::getOpenFileName(mMainWindow,
                                               "Link File",
                                               "",
                                               "AniVect Files (*.av)");
    if(!src.isEmpty()) {
        setSrc(src);
    }
}

void ExternalLinkBox::setSrc(const QString &src) {
    mSrc = src;
    setName("Link " + src);
    reload();
}

QPointF InternalLinkBox::getRelCenterPosition() {
    return mLinkTarget->getRelCenterPosition();
}

qreal InternalLinkBox::getEffectsMargin() {
    return mLinkTarget->getEffectsMargin();
}

BoundingBox *InternalLinkBox::getLinkTarget() {
    return mLinkTarget;
}

BoundingBox *InternalLinkBox::createLink(BoxesGroup *parent) {
    return mLinkTarget->createLink(parent);
}

BoundingBox *InternalLinkBox::createSameTransformationLink(BoxesGroup *parent) {
    return mLinkTarget->createSameTransformationLink(parent);
}

void InternalLinkBox::scheduleAwaitUpdateSLOT() {
    scheduleSoftUpdate();
}

void InternalLinkBox::updateRelBoundingRect() {
    mRelBoundingRect = mLinkTarget->getRelBoundingRect();
    BoundingBox::updateRelBoundingRect();
}

InternalLinkBox::InternalLinkBox(BoxesGroup *parent) :
    BoundingBox(parent, TYPE_INTERNAL_LINK) {
}

InternalLinkBox::InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent) :
    InternalLinkBox(parent) {
    setLinkTarget(linkTarget);
}

QImage InternalLinkBox::getAllUglyPixmapProvidedTransform(
                                const qreal &effectsMargin,
                                const qreal &resolution,
                                const QMatrix &allUglyTransform,
                                QPoint *draWPosP) {
    return mLinkTarget->getAllUglyPixmapProvidedTransform(effectsMargin,
                                                          resolution,
                                                          allUglyTransform,
                                                          draWPosP);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &point)
{
    return mLinkTarget->relPointInsidePath(point);
}

SameTransformInternalLink::SameTransformInternalLink(BoxesGroup *&parent) :
    InternalLinkBox(parent) {

}

SameTransformInternalLink::SameTransformInternalLink(BoundingBox *linkTarget,
                                                     BoxesGroup *parent) :
    InternalLinkBox(linkTarget, parent) {
}

void SameTransformInternalLink::updateCombinedTransform() {
    if(mLinkTarget == NULL) return;
//    mCombinedTransformMatrix =
//            mLinkTarget->getRelativeTransform()*
//            mParent->getCombinedTransform();


    updateAfterCombinedTransformationChanged();

    scheduleSoftUpdate();
}

QMatrix SameTransformInternalLink::getRelativeTransform() const {
    if(mLinkTarget == NULL) return QMatrix();
    return mLinkTarget->getRelativeTransform();
}

const QPainterPath &SameTransformInternalLink::getRelBoundingRectPath() {
    if(mLinkTarget == NULL) return QPainterPath();
    return mLinkTarget->getRelBoundingRectPath();
}

qreal SameTransformInternalLink::getEffectsMargin() {
    if(mLinkTarget == NULL) return 0.;
    return mLinkTarget->getEffectsMargin();
}

SameTransformInternalLinkBoxesGroup::SameTransformInternalLinkBoxesGroup(
                                                BoxesGroup *parent) :
    InternalLinkBoxesGroup(parent) {

}

SameTransformInternalLinkBoxesGroup::SameTransformInternalLinkBoxesGroup(
                                                BoxesGroup *linkTarget,
                                                BoxesGroup *parent) :
    InternalLinkBoxesGroup(linkTarget, parent) {

}

void SameTransformInternalLinkBoxesGroup::updateCombinedTransform() {
//    mCombinedTransformMatrix = mLinkTarget->getRelativeTransform()*
//            mParent->getCombinedTransform();


    updateAfterCombinedTransformationChanged();

    scheduleSoftUpdate();
}

QMatrix SameTransformInternalLinkBoxesGroup::getRelativeTransform() const {
    return mLinkTarget->getRelativeTransform();
}

const QPainterPath &SameTransformInternalLinkBoxesGroup::getRelBoundingRectPath() {
    return mLinkTarget->getRelBoundingRectPath();
}

qreal SameTransformInternalLinkBoxesGroup::getEffectsMargin() {
    return mLinkTarget->getEffectsMargin();
}

void InternalLinkCanvas::updateRelBoundingRect() {
    if(mClipToCanvasSize) {
        //        QPainterPath boundingPaths = QPainterPath();
        //        Q_FOREACH(BoundingBox *child, mChildren) {
        //            boundingPaths.addPath(
        //                        child->getRelativeTransform().
        //                        map(child->getRelBoundingRectPath()));
        //        }
        mRelBoundingRect =
                QRectF(QPointF(0., 0.),
                       ((Canvas*)mLinkTarget.data())->getCanvasSize());
        //boundingPaths.boundingRect();

        BoundingBox::updateRelBoundingRect();
    } else {
        BoxesGroup::updateRelBoundingRect();
    }
}

void InternalLinkCanvas::setClippedToCanvasSize(const bool &clipped) {
    mClipToCanvasSize = clipped;
    scheduleSoftUpdate();
}

void InternalLinkCanvas::draw(QPainter *p) {
    p->save();
    if(mClipToCanvasSize) {
        p->setClipRect(mRelBoundingRect);
    }
    p->setTransform(QTransform(getCombinedTransform().inverted()), true);
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        //box->draw(p);
        box->drawPixmap(p);
    }

    p->restore();
}
