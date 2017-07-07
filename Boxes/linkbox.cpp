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

InternalLinkBox::InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent) :
    BoundingBox(parent, TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &point)
{
    return mLinkTarget->relPointInsidePath(point);
}

SameTransformInternalLink::SameTransformInternalLink(BoundingBox *linkTarget,
                                                     BoxesGroup *parent) :
    InternalLinkBox(linkTarget, parent) {
}

QMatrix SameTransformInternalLink::getRelativeTransform() const {
    return mLinkTarget->getRelativeTransform();
}

const SkPath &SameTransformInternalLink::getRelBoundingRectPath() {
    return mLinkTarget->getRelBoundingRectPath();
}

qreal SameTransformInternalLink::getEffectsMargin() {
    return mLinkTarget->getEffectsMargin();
}

SameTransformInternalLinkBoxesGroup::SameTransformInternalLinkBoxesGroup(
                                                BoxesGroup *linkTarget,
                                                BoxesGroup *parent) :
    InternalLinkBoxesGroup(linkTarget, parent) {

}

QMatrix SameTransformInternalLinkBoxesGroup::getRelativeTransform() const {
    return mLinkTarget->getRelativeTransform();
}

const SkPath &SameTransformInternalLinkBoxesGroup::getRelBoundingRectPath() {
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
        QSize size = ((Canvas*)mLinkTarget.data())->getCanvasSize();
        mRelBoundingRect = QRectF(0., 0.,
                                  size.width(), size.height());
        mRelBoundingRectSk = QRectFToSkRect(mRelBoundingRect);
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

//void InternalLinkCanvas::draw(QPainter *p) {
//    p->save();
//    if(mClipToCanvasSize) {
//        p->setClipRect(mRelBoundingRect);
//    }
//    p->setTransform(QTransform(getCombinedTransform().inverted()), true);
//    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
//        box->drawPixmap(p);
//    }

//    p->restore();
//}
