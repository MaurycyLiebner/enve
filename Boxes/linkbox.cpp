#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"

ExternalLinkBox::ExternalLinkBox(QString srcFile, BoxesGroup *parent) :
    BoxesGroup(parent) {
    mSrc = srcFile;
    reload();
    setType(TYPE_LINK);
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

void InternalLinkBox::updateBoundingRect() {
    mRelBoundingRect = mLinkTarget->getRelBoundingRect();
    qreal effectsMargin = mLinkTarget->getEffectsMargin()*
                          mUpdateCanvasTransform.m11();
    mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                        adjusted(-effectsMargin, -effectsMargin,
                                 effectsMargin, effectsMargin);

    BoundingBox::updateBoundingRect();
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
