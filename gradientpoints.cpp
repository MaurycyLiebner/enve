#include "gradientpoints.h"

GradientPoints::GradientPoints() : ComplexAnimator()
{
    prp_setName("gradient points");
}

GradientPoints::~GradientPoints()
{
    startPoint->decNumberPointers();
    endPoint->decNumberPointers();
}

void GradientPoints::initialize(PathBox *parentT)
{
    parent = parentT;
    startPoint = new GradientPoint(parent);
    startPoint->incNumberPointers();
    ca_addChildAnimator(startPoint->getRelativePosAnimatorPtr() );
    startPoint->getRelativePosAnimatorPtr()->prp_setName("point1");
    endPoint = new GradientPoint(parent);
    endPoint->getRelativePosAnimatorPtr()->prp_setName("point2");
    endPoint->incNumberPointers();
    ca_addChildAnimator(endPoint->getRelativePosAnimatorPtr() );
    enabled = false;
}

#include <QSqlError>
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

void GradientPoints::prp_makeDuplicate(Property *target) {
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

void GradientPoints::drawGradientPoints(QPainter *p) {
    if(enabled) {
        p->setPen(QPen(Qt::black, 1.5));
        p->drawLine(startPoint->getAbsolutePos(), endPoint->getAbsolutePos());
        p->setPen(QPen(Qt::white, 0.75));
        p->drawLine(startPoint->getAbsolutePos(), endPoint->getAbsolutePos());
        startPoint->draw(p);
        endPoint->draw(p);
    }
}

MovablePoint *GradientPoints::qra_getPointAt(const QPointF &absPos)
{
    if(enabled) {
        if(startPoint->isPointAtAbsPos(absPos) ) {
            return startPoint;
        } else if (endPoint->isPointAtAbsPos(absPos) ){
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
