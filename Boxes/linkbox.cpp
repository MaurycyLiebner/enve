#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"
#include "canvas.h"

ExternalLinkBox::ExternalLinkBox() :
    BoxesGroup() {
    setType(TYPE_EXTERNAL_LINK);
    setName("Link Empty");
}

void ExternalLinkBox::reload() {
    QSqlDatabase db = QSqlDatabase::database();//not dbConnection
    db.setDatabaseName(mSrc);
    db.open();

    loadChildrenFromSql(0, false);

    db.close();

    scheduleUpdate();
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
    return mLinkTarget.data();
}

BoundingBox *InternalLinkBox::createLink() {
    return mLinkTarget->createLink();
}

BoundingBoxRenderData *InternalLinkBox::createRenderData() {
    return mLinkTarget->createRenderData();
}

void InternalLinkBox::setupBoundingBoxRenderDataForRelFrame(
        const int &relFrame, BoundingBoxRenderData *data) {
    mLinkTarget->setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
}

void InternalLinkBox::scheduleAwaitUpdateSLOT() {
    scheduleUpdate();
}

InternalLinkBox::InternalLinkBox(BoundingBox *linkTarget) :
    BoundingBox(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &point)
{
    return mLinkTarget->relPointInsidePath(point);
}

void InternalLinkCanvas::setClippedToCanvasSize(const bool &clipped) {
    mClipToCanvasSize->setValue(clipped);
    scheduleUpdate();
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
