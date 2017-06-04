#include "gradientpoints.h"
#include "gradientpoint.h"
#include "skqtconversions.h"
#include <QSqlError>

GradientPoints::GradientPoints() : ComplexAnimator()
{
    prp_setName("gradient points");
}

GradientPoints::~GradientPoints() {
}

void GradientPoints::initialize(PathBox *parentT)
{
    parent = parentT;
    startPoint = new GradientPoint(parent);
    ca_addChildAnimator(startPoint );
    startPoint->prp_setName("point1");
    endPoint = new GradientPoint(parent);
    endPoint->prp_setName("point2");
    ca_addChildAnimator(endPoint);
    enabled = false;
}

void GradientPoints::prp_loadFromSql(const int &identifyingId) {

    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM gradientpoints WHERE id = %1").
            arg(identifyingId);
    if(query.exec(queryStr) ) {
        query.next();
        endPoint->prp_loadFromSql(query.value("endpointid").toInt());
        startPoint->prp_loadFromSql(query.value("startpointid").toInt());
    } else {
        qDebug() << "Could not load gradientpoints with id " << identifyingId;
    }
}

int GradientPoints::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    int startPtId = startPoint->prp_saveToSql(query);
    int endPtId = endPoint->prp_saveToSql(query);
    if(!query->exec(QString("INSERT INTO gradientpoints (endpointid, "
                            "startpointid) "
                "VALUES (%1, %2)").
                arg(endPtId).
                arg(startPtId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    return query->lastInsertId().toInt();

}

void GradientPoints::duplicatePointsFrom(GradientPoint *startPointT,
                                         GradientPoint *endPointT) {
    startPointT->makeDuplicate(startPoint);
    endPointT->makeDuplicate(endPoint);
}

void GradientPoints::makeDuplicate(Property *target) {
    GradientPoints *gradientPointsTarget = (GradientPoints*)target;
    gradientPointsTarget->duplicatePointsFrom(startPoint,
                                              endPoint);
}

void GradientPoints::enable() {
    if(enabled) {
        return;
    }
    enabled = true;
}

void GradientPoints::setPositions(QPointF startPos,
                                  QPointF endPos, bool saveUndoRedo) {
    startPoint->setRelativePos(startPos, saveUndoRedo);
    endPoint->setRelativePos(endPos, saveUndoRedo);
}

void GradientPoints::disable()
{
    enabled = false;
}

void GradientPoints::drawGradientPointsSk(SkCanvas *canvas,
                                          const SkScalar &invScale) {
    if(enabled) {
        SkPoint startPos = QPointFToSkPoint(startPoint->getAbsolutePos());
        SkPoint endPos = QPointFToSkPoint(endPoint->getAbsolutePos());
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        paint.setStrokeWidth(1.5*invScale);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->drawLine(startPos, endPos, paint);
        paint.setColor(SK_ColorWHITE);
        paint.setStrokeWidth(0.75*invScale);
        canvas->drawLine(startPos, endPos, paint);
        startPoint->drawSk(canvas, invScale);
        endPoint->drawSk(canvas, invScale);
    }
}

MovablePoint *GradientPoints::qra_getPointAt(const QPointF &absPos,
                                             const qreal &canvasScaleInv) {
    if(enabled) {
        if(startPoint->isPointAtAbsPos(absPos, canvasScaleInv) ) {
            return startPoint;
        } else if (endPoint->isPointAtAbsPos(absPos, canvasScaleInv) ){
            return endPoint;
        }
    }
    return NULL;
}

QPointF GradientPoints::getStartPoint()
{
    return startPoint->getRelativePos();
}

QPointF GradientPoints::getEndPoint()
{
    return endPoint->getRelativePos();
}

void GradientPoints::setColors(QColor startColor, QColor endColor)
{
    startPoint->setColor(startColor);
    endPoint->setColor(endColor);
}
