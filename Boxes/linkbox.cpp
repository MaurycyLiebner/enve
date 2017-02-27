#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"

ExternalLinkBox::ExternalLinkBox(BoxesGroup *parent) :
    BoxesGroup(parent) {
    setType(TYPE_EXTERNAL_LINK);
    setName("Link Empty");
}

void ExternalLinkBox::reload() {
    foreach(BoundingBox *box, mChildBoxes) {
        box->decNumberPointers();
    }

    QSqlDatabase db = QSqlDatabase::database();//not dbConnection
    db.setDatabaseName(mSrc);
    db.open();

    loadChildrenFromSql(0, false);

    db.close();

    scheduleAwaitUpdate();
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
    scheduleAwaitUpdate();
}

void InternalLinkBox::updateBoundingRect() {
    mRelBoundingRect = mLinkTarget->getRelBoundingRect();
    qreal effectsMargin = mLinkTarget->getEffectsMargin()*
                          mUpdateCanvasTransform.m11();
    mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                        adjusted(-effectsMargin, -effectsMargin,
                                 effectsMargin, effectsMargin);

    BoundingBox::updateBoundingRect();
}

InternalLinkBox::InternalLinkBox(BoxesGroup *parent) :
    BoundingBox(parent, TYPE_INTERNAL_LINK) {
}

InternalLinkBox::InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent) :
    InternalLinkBox(parent) {
    setLinkTarget(linkTarget);
}

QImage InternalLinkBox::renderPreviewProvidedTransform(
                                const qreal &effectsMargin,
                                const qreal &resolutionScale,
                                const QMatrix &renderTransform,
                                QPointF *drawPos) {
    return mLinkTarget->renderPreviewProvidedTransform(effectsMargin,
                                                       resolutionScale,
                                                   renderTransform,
                                                   drawPos);
}

QImage InternalLinkBox::getAllUglyPixmapProvidedTransform(
                                const qreal &effectsMargin,
                                const QMatrix &allUglyTransform,
                                QRectF *allUglyBoundingRectP) {
    return mLinkTarget->getAllUglyPixmapProvidedTransform(effectsMargin,
                                                          allUglyTransform,
                                                          allUglyBoundingRectP);
}

QImage InternalLinkBox::getPrettyPixmapProvidedTransform(
                                const QMatrix &transform,
                                QRectF *pixBoundingRectClippedToViewP) {
    return mLinkTarget->getPrettyPixmapProvidedTransform(
                                        transform,
                                        pixBoundingRectClippedToViewP);
}

void InternalLinkBox::drawSelected(QPainter *p,
                                   const CanvasMode &)
{
    if(mVisible) {
        p->save();

        //        QPainterPath mapped;
        //        mapped.addRect(mPixmap.rect());
        //        mapped = mCombinedTransformMatrix.map(mapped);
        //        QPen pen = p->pen();
        //        p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
//        p->setBrush(Qt::NoBrush);
//        p->drawPath(mapped);
//        p->setPen(pen);

        drawBoundingRect(p);
        p->restore();
    }
}

bool InternalLinkBox::relPointInsidePath(QPointF point)
{
    return mLinkTarget->relPointInsidePath(point);
}

SameTransformInternalLink::SameTransformInternalLink(
        BoxesGroup *parent) :
    InternalLinkBox(parent) {

}

SameTransformInternalLink::SameTransformInternalLink(BoundingBox *linkTarget,
                                                     BoxesGroup *parent) :
    InternalLinkBox(linkTarget, parent) {
}

void SameTransformInternalLink::updateCombinedTransform() {
    if(mLinkTarget == NULL) return;
    mCombinedTransformMatrix = mLinkTarget->getRelativeTransform()*
            mParent->getCombinedTransform();


    updateAfterCombinedTransformationChanged();

    scheduleAwaitUpdate();
    updateUglyPaintTransform();
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
        BoxesGroup *linkTarget, BoxesGroup *parent) :
    InternalLinkBoxesGroup(linkTarget, parent) {

}

void SameTransformInternalLinkBoxesGroup::updateCombinedTransform() {
    mCombinedTransformMatrix = mLinkTarget->getRelativeTransform()*
            mParent->getCombinedTransform();


    updateAfterCombinedTransformationChanged();

    scheduleAwaitUpdate();
    updateUglyPaintTransform();
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

void InternalLinkCanvas::updateBoundingRect() {
    if(mClipToCanvasSize) {
        //        QPainterPath boundingPaths = QPainterPath();
        //        foreach(BoundingBox *child, mChildren) {
        //            boundingPaths.addPath(
        //                        child->getRelativeTransform().
        //                        map(child->getRelBoundingRectPath()));
        //        }
        mRelBoundingRect = QRectF(QPointF(0., 0.),
                                  ((Canvas*)mLinkTarget)->getCanvasSize());
        //boundingPaths.boundingRect();

        qreal effectsMargin = mEffectsMargin*
                mUpdateCanvasTransform.m11();

        mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                adjusted(-effectsMargin, -effectsMargin,
                         effectsMargin, effectsMargin);

        BoundingBox::updateBoundingRect();
    } else {
        BoxesGroup::updateBoundingRect();
    }
}

void InternalLinkCanvas::setClippedToCanvasSize(const bool &clipped) {
    mClipToCanvasSize = clipped;
    scheduleAwaitUpdate();
}

void InternalLinkCanvas::draw(QPainter *p)
{
    if(mVisible) {
        p->save();
        p->setTransform(QTransform(mCombinedTransformMatrix.inverted()), true);
        if(mClipToCanvasSize) {
            p->setClipPath(mMappedBoundingRectPath);
            //p->setClipRect(mRelBoundingRect);
        }
        foreach(BoundingBox *box, mChildBoxes) {
            //box->draw(p);
            box->drawPixmap(p);
        }

        p->restore();
    }
}

void InternalLinkCanvas::drawForPreview(QPainter *p) {
    if(mVisible) {
        p->save();
        p->setTransform(QTransform(
                            mCombinedTransformMatrix.inverted()),
                            true);
        if(mClipToCanvasSize) {
            p->setClipPath(mMappedBoundingRectPath);
            //p->setClipRect(mRelBoundingRect);
        }
        foreach(BoundingBox *box, mChildBoxes) {
            //box->draw(p);
            box->drawPreviewPixmap(p);
        }

        p->restore();
    }
}

