#include "bonepoint.h"

const int MAP_SIZE = 100;

BonePoint::BonePoint(BoundingBox *bonesBox) :
    MovablePoint(bonesBox, TYPE_BONE_POINT) {

}

BonePoint::BonePoint(BoundingBox *bonesBox,
                     BonePoint *parentBonePt) :
    BonePoint(bonesBox) {
    setParentBonePoint(parentBonePt);
}

void BonePoint::addChildBonePoint(BonePoint *childBonePt) {
    mChildBonePoints << childBonePt->ref<BonePoint>();
}

void BonePoint::setParentBonePointVar(BonePoint *parentPoint) {
    mParentBonePoint = parentPoint->ref<BonePoint>();
}

void BonePoint::setParentBonePoint(BonePoint *parentBonePt) {
    setParentBonePointVar(parentBonePt);
    parentBonePt->addChildBonePoint(this);
}

void WeightMap::setWeightImageColorTable(QImage *img) {
    img->setColorCount(256);
    for(int i = 0; i <= 15; i++) {
        img->setColor(i, QColor::fromHsv(255 - i, 255, 255, i*255/15).rgba());
    }
    for(int i = 16; i <= 255; i++) {
        img->setColor(i, QColor::fromHsv(255 - i, 255, 255).rgb());
    }
}

void WeightMap::paintAt(const qreal &x,
                        const qreal &y,
                        const qreal &radius,
                        const qreal &weight,
                        const qreal &hardness,
                        const qreal &opacity,
                        const WeightMap::PaintMode &paintMode) {
    int minX = qMax(0, qFloor(x - radius));
    int minY = qMax(0, qFloor(x - radius));
    int maxX = qMin(mWeightMapImg.width() - 1, qCeil(x + radius));
    int maxY = qMin(mWeightMapImg.height() - 1, qCeil(y + radius));

    for(int i = minX; i <= maxX; i++) {
        for(int j = minY; j <= maxY; j++) {
            qreal dX = i - x;
            qreal dY = j - y;
            qreal dd = (dX*dX + dY*dY)/(radius*radius);
            qreal opa;
            if(dd > 1.) {
                continue;
            } else if(dd < hardness) {
                opa = dd + 1. - dd/hardness;
            } else {
                opa = hardness/(1. - hardness)*(1. - dd);
            }
            qreal pixOpacity = opa * opacity;
            qreal value;
            if(paintMode == PAINT_ADD) {
                value = mWeightMapImg.pixelIndex(i, j) + pixOpacity * weight;
            } else if(paintMode == PAINT_SUBSTRACT) {
                value = mWeightMapImg.pixelIndex(i, j) - pixOpacity * weight;
            } else { // if(paintMode == PAINT_REPLACE) {
                value = mWeightMapImg.pixelIndex(i, j)*(1. - pixOpacity) +
                                pixOpacity * weight;
            }
            mWeightMapImg.setPixel(i, j, clamp((int)value, 0, 255));
        }
    }
}

void WeightMap::setSize(const int &width, const int &height) {
    mWeightMapImg = QImage(width, height, QImage::Format_Indexed8);
    setWeightImageColorTable(&mWeightMapImg);
}



void PathWeightMap::setNumberOfControlPoints(const int &nPoints) {
    if(mControlPointsWeight != NULL) delete[] mControlPointsWeight;
    mNumberOfControlPoints = nPoints;
    mControlPointsWeight = new qreal[nPoints];
    for(int i = 0; i < nPoints; i++) {
        mControlPointsWeight[i] = 0.;
    }
}

void PathWeightMap::setPointWeight(const int &pointId,
                                   const qreal &weight) {
    mControlPointsWeight[pointId] = weight;
}

void PathWeightMap::applyToPath(QPainterPath *path) {
    QPainterPath newPath;


    *path = newPath;
}

void PathWeightMap::startPath(const QPointF &p) {
    mPath.moveTo(p);
    mLastPoint = p;
}
#include "edge.h"
void PathWeightMap::addPoint(const QPointF &c1,
                             const QPointF &c2,
                             const QPointF &p) {
    QPointF sp1 = mLastPoint;
    QPointF sc1 = c1;
    QPointF sc2;
    QPointF sp2;
    QPointF sc3;
    QPointF sc4 = c2;
    QPointF sp3 = p;
//    Edge::getNewRelPosForKnotInsertionAtT(
//                sp1, &sc1, &sc4, sp3,
//                &sp2, &sc2, &sc3,
//                tVal);
}

QPainterPath PathWeightMap::finishPath() {
    return mPath;
}
