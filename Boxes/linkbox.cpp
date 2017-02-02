#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"

ExternalLinkBox::ExternalLinkBox(QString srcFile, BoxesGroup *parent) :
    BoxesGroup(parent) {

    mSrc = srcFile;
    reload();
    setType(TYPE_EXTERNAL_LINK);
    setName("Link " + srcFile);
}

void ExternalLinkBox::reload() {
    foreach(BoundingBox *box, mChildren) {
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

InternalLinkBox::InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent) :
    BoundingBox(parent, TYPE_INTERNAL_LINK) {
    mLinkTarget = linkTarget;
    connect(linkTarget, SIGNAL(scheduleAwaitUpdateAllLinkBoxes()),
            this, SLOT(scheduleAwaitUpdateSLOT()));
}

QPixmap InternalLinkBox::renderPreviewProvidedTransform(
                                const qreal &effectsMargin,
                                const QMatrix &renderTransform,
                                QPointF *drawPos) {
    return mLinkTarget->renderPreviewProvidedTransform(effectsMargin,
                                                   renderTransform,
                                                   drawPos);
}

QPixmap InternalLinkBox::getAllUglyPixmapProvidedTransform(
                                const qreal &effectsMargin,
                                const QMatrix &allUglyTransform,
                                QRectF *allUglyBoundingRectP) {
    return mLinkTarget->getAllUglyPixmapProvidedTransform(effectsMargin,
                                                          allUglyTransform,
                                                          allUglyBoundingRectP);
}

QPixmap InternalLinkBox::getPrettyPixmapProvidedTransform(
                                const QMatrix &transform,
                                QRectF *pixBoundingRectClippedToViewP) {
    return mLinkTarget->getPrettyPixmapProvidedTransform(
                                        transform,
                                        pixBoundingRectClippedToViewP);
}

void InternalLinkBox::drawSelected(QPainter *p, CanvasMode)
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

SameTransformInternalLink::SameTransformInternalLink(BoundingBox *linkTarget, BoxesGroup *parent) :
    InternalLinkBox(linkTarget, parent) {

}

void SameTransformInternalLink::updateCombinedTransform() {
    if(mParent == NULL) {
        updateAfterCombinedTransformationChanged();
    } else {
        mCombinedTransformMatrix = mLinkTarget->getRelativeTransform()*
                mParent->getCombinedTransform();


        updateAfterCombinedTransformationChanged();

        scheduleAwaitUpdate();
        updateUglyPaintTransform();
    }
}

QMatrix SameTransformInternalLink::getRelativeTransform() const {
    return mLinkTarget->getRelativeTransform();
}

const QPainterPath &SameTransformInternalLink::getRelBoundingRectPath() {
    return mLinkTarget->getRelBoundingRectPath();
}

qreal SameTransformInternalLink::getEffectsMargin() {
    return mLinkTarget->getEffectsMargin();
}

SameTransformInternalLinkBoxesGroup::SameTransformInternalLinkBoxesGroup(BoxesGroup *linkTarget, BoxesGroup *parent) :
    InternalLinkBoxesGroup(linkTarget, parent) {

}

void SameTransformInternalLinkBoxesGroup::updateCombinedTransform() {
    if(mParent == NULL) {
        updateAfterCombinedTransformationChanged();
    } else {
        mCombinedTransformMatrix = mLinkTarget->getRelativeTransform()*
                mParent->getCombinedTransform();


        updateAfterCombinedTransformationChanged();

        scheduleAwaitUpdate();
        updateUglyPaintTransform();
    }
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
        foreach(BoundingBox *box, mChildren) {
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
        foreach(BoundingBox *box, mChildren) {
            //box->draw(p);
            box->drawPreviewPixmap(p);
        }

        p->restore();
    }
}

