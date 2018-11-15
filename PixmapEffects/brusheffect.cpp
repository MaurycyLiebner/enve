#include "brusheffect.h"
#include "Animators/intanimator.h"
#include "Paint/PaintLib/surface.h"
#include "Properties/boolproperty.h"

BrushEffect::BrushEffect(qreal numberStrokes,
                         qreal brushMinRadius, qreal brushMaxRadius,
                         qreal strokeMaxLength,
                         qreal strokeMinDirectionAngle,
                         qreal strokeMaxDirectionAngle,
                         qreal strokeCurvature) : PixmapEffect(EFFECT_BRUSH) {
    prp_setName("brush");

    mNumberStrokes = (new QrealAnimator())->ref<QrealAnimator>();
    mNumberStrokes->qra_setCurrentValue(numberStrokes);
    mNumberStrokes->prp_setName("number strokes");
    mNumberStrokes->qra_setValueRange(0., 9999.);
    ca_addChildAnimator(mNumberStrokes.data());

    mMinBrushRadius = (new QrealAnimator())->ref<QrealAnimator>();
    mMinBrushRadius->qra_setCurrentValue(brushMinRadius);
    mMinBrushRadius->prp_setName("min radius");
    mMinBrushRadius->qra_setValueRange(0., 999.);
    ca_addChildAnimator(mMinBrushRadius.data());

    mMaxBrushRadius = (new QrealAnimator())->ref<QrealAnimator>();
    mMaxBrushRadius->qra_setCurrentValue(brushMaxRadius);
    mMaxBrushRadius->prp_setName("max radius");
    mMaxBrushRadius->qra_setValueRange(0., 999.);
    ca_addChildAnimator(mMaxBrushRadius.data());

    mStrokeMaxLength = (new QrealAnimator())->ref<QrealAnimator>();
    mStrokeMaxLength->qra_setCurrentValue(strokeMaxLength);
    mStrokeMaxLength->prp_setName("max stroke length");
    mStrokeMaxLength->qra_setValueRange(0., 9999.);
    ca_addChildAnimator(mStrokeMaxLength.data());

    mStrokeMinDirectionAngle = (new QrealAnimator())->ref<QrealAnimator>();
    mStrokeMinDirectionAngle->qra_setCurrentValue(strokeMinDirectionAngle);
    mStrokeMinDirectionAngle->prp_setName("min stroke angle");
    mStrokeMinDirectionAngle->qra_setValueRange(0., 360.);
    ca_addChildAnimator(mStrokeMinDirectionAngle.data());

    mStrokeMaxDirectionAngle = (new QrealAnimator())->ref<QrealAnimator>();
    mStrokeMaxDirectionAngle->qra_setCurrentValue(strokeMaxDirectionAngle);
    mStrokeMaxDirectionAngle->prp_setName("max stroke angle");
    mStrokeMaxDirectionAngle->qra_setValueRange(0., 360.);
    ca_addChildAnimator(mStrokeMaxDirectionAngle.data());

    mStrokeCurvature = (new QrealAnimator())->ref<QrealAnimator>();
    mStrokeCurvature->qra_setCurrentValue(strokeCurvature);
    mStrokeCurvature->prp_setName("curvature");
    mStrokeCurvature->qra_setValueRange(0., 1.);
    ca_addChildAnimator(mStrokeCurvature.data());

    mSmoothness = (new QrealAnimator())->ref<QrealAnimator>();
    mRandomize = (new BoolProperty())->ref<BoolProperty>();
    mRandomizeStep = (new IntAnimator())->ref<IntAnimator>();
    mSmoothTransform = (new BoolProperty())->ref<BoolProperty>();
    mSeed = (new IntAnimator())->ref<IntAnimator>();

    mSmoothness->prp_setName("smoothness");
    mSmoothness->qra_setValueRange(0., 1.);

    mRandomize->prp_setName("randomize");

    mRandomizeStep->prp_setName("rand frame step");
    mRandomizeStep->setIntValueRange(1, 99);

    mSmoothTransform->prp_setName("smooth progression");

    mSeed->prp_setName("seed");
    mSeed->setIntValueRange(0, 9999);
    mSeed->setCurrentIntValue(qrand() % 9999, false);

    ca_addChildAnimator(mSmoothness.data());
    ca_addChildAnimator(mRandomize.data());
    ca_addChildAnimator(mRandomizeStep.data());
    ca_addChildAnimator(mSmoothTransform.data());
    ca_addChildAnimator(mSeed.data());
}

PixmapEffectRenderData *BrushEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, const std::shared_ptr<BoundingBoxRenderData>& ) {
    BrushEffectRenderData *renderData = new BrushEffectRenderData();
    renderData->maxBrushRadius =
            mMaxBrushRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->minBrushRadius =
            mMinBrushRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->numberStrokes =
            mNumberStrokes->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->strokeCurvature =
            mStrokeCurvature->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->strokeMaxDirectionAngle =
            mStrokeMaxDirectionAngle->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->strokeMaxLength =
            mStrokeMaxLength->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->strokeMinDirectionAngle =
            mStrokeMinDirectionAngle->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->smooth =
            mSmoothTransform->getValue();
    renderData->randomize =
            mRandomize->getValue();
    renderData->randStep =
            mRandomizeStep->getCurrentIntValueAtRelFrameF(relFrame);
    renderData->relFrame = relFrame;
    renderData->seed = mSeed->getCurrentIntEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

void BrushEffect::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical, int *lastIdentical, const int &relFrame) {
    if(mRandomize->getValue()) {
        if(mSmoothTransform->getValue()) {
            *firstIdentical = relFrame;
            *lastIdentical = relFrame;
        } else {
            int frameStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
            *firstIdentical = relFrame - relFrame % frameStep;
            *lastIdentical = *firstIdentical + frameStep;
        }
    } else {
        PixmapEffect::prp_getFirstAndLastIdenticalRelFrame(firstIdentical,
                                                           lastIdentical,
                                                           relFrame);
    }
}

bool BrushEffect::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                  const int &relFrame2) {
    if(mRandomize->getValue()) {
        int frameStep1 = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame1);
        int frameStep2 = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame2);
        return relFrame1 - relFrame1 % frameStep1 ==
                relFrame2 - relFrame2 % frameStep2;
    }
    return PixmapEffect::prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2);
}

void BrushEffect::prp_setAbsFrame(const int &frame) {
    ComplexAnimator::prp_setAbsFrame(frame);
    if(mRandomize->getValue()) {
        prp_callUpdater();
    }
}

qreal BrushEffect::getMargin() {
    return mMaxBrushRadius->qra_getCurrentEffectiveValue();
}

qreal BrushEffect::getMarginAtRelFrame(const int &relFrame) {
    return mMaxBrushRadius->qra_getEffectiveValueAtRelFrame(relFrame);
}

BrushStroke::BrushStroke(const QPointF &startPos,
                         const QPointF &startCtrlPos,
                         const QPointF &endCtrlPos,
                         const QPointF &endPos,
                         const qreal &radius,
                         const QColor &color) {
    mStartPos = startPos;
    mStartCtrlPos = startCtrlPos;
    mEndCtrlPos = endCtrlPos;
    mEndPos = endPos;
    mRadius = radius;
    mColor = color;
}

void BrushStroke::interpolateWith(const QPointF &startPos, const QPointF &startCtrlPos, const QPointF &endCtrlPos, const QPointF &endPos, const qreal &radius, const QColor &color, const qreal &weight) {
    mStartPos = mStartPos*(1. - weight) + startPos*weight;
    mStartCtrlPos = mStartCtrlPos*(1. - weight) + startCtrlPos*weight;
    mEndCtrlPos = mEndCtrlPos*(1. - weight) + endCtrlPos*weight;
    mEndPos = mEndPos*(1. - weight) + endPos*weight;
    mRadius = mRadius*(1. - weight) + radius*weight;
    qreal red = mColor.redF()*(1. - weight) + color.redF()*weight;
    qreal green = mColor.greenF()*(1. - weight) + color.greenF()*weight;
    qreal blue = mColor.blueF()*(1. - weight) + color.blueF()*weight;
    qreal alpha = mColor.alphaF()*(1. - weight) + color.alphaF()*weight;
    mColor.setRgbF(red, green, blue, alpha);
}

QColor colorMix(QColor col1, QColor col2) {
    qreal a1 = col1.alphaF();
    qreal a2 = col2.alphaF();
    qreal aSum = a1 + a2;
    if(aSum < 0.001) return QColor(Qt::transparent);
    return QColor((col1.red()*a1 + col2.red()*a2)/aSum,
                  (col1.green()*a1 + col2.green()*a2)/aSum,
                  (col1.blue()*a1 + col2.blue()*a2)/aSum,
                  qMin(a1, a2)*255);
}

bool isTooDifferent(const QColor &col1, const QColor &col2) {
    qreal val1 = col1.alphaF();
    qreal val2 = col2.alphaF();
    if(qAbs(val1 - val2) > 0.2) return true;
    return false;
}

#include <QDebug>
void BrushStroke::prepareToDrawOnImage(const SkBitmap &img) {
    mStrokePath = QPainterPath(mStartPos);
    mStrokePath.cubicTo(mStartCtrlPos, mEndCtrlPos, mEndPos);

    QPainterPathStroker stroker;
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setWidth(2*mRadius);
    mWholeStrokePath = stroker.createStroke(mStrokePath);
    mBoundingRect = mWholeStrokePath.boundingRect();

    qreal currLen = 0.;
    qreal lastRadAngle = 0.;
    qreal maxLen = mStrokePath.length();
    int width = img.width();
    int height = img.height();
    while(currLen < maxLen) {
        qreal perc = mStrokePath.percentAtLength(currLen);

        QPointF colPos = mStrokePath.pointAtPercent(perc);
        SkColor colSkAtPix = img.getColor(clampInt(colPos.x(), 0, width - 1),
                                          clampInt(colPos.y(), 0, height - 1));
        QColor colAtPix = QColor(SkColorGetR(colSkAtPix),
                                 SkColorGetG(colSkAtPix),
                                 SkColorGetB(colSkAtPix),
                                 SkColorGetA(colSkAtPix));
        if(isTooDifferent(colAtPix, mColor)) {
            break;
        }
        QColor col = colorMix(colAtPix, mColor);

        qreal slope = mStrokePath.slopeAtPercent(perc);
        qreal atanVal = atan(slope);
        qreal radAngle = atanVal;
        while(qAbs(radAngle - lastRadAngle) > M_PI*0.5) {
            if(radAngle > lastRadAngle) {
                radAngle -= M_PI;
            } else {
                radAngle += M_PI;
            }
        }
        lastRadAngle = radAngle;
        qreal degAngle = radAngle * 180 / M_PI;

        qreal hardness = 0.5; //mBrush->getHardness()
        qreal opacity = 0.5; //mBrush->getOpacity()
        qreal aspectRatio = 1.; //mBrush->getAspectRatio()
        //qreal radius = 10.; //mBrush->getRadius()
        mDabs << Dab(colPos.x(), colPos.y(),
                     hardness, opacity,
                     aspectRatio, mRadius,
                     degAngle, col.redF(), col.greenF(), col.blueF(),
                     false);
        currLen += mRadius*0.5;//mBrush->getDistBetweenDabsPx();
    }
}

void BrushStroke::drawOnImage(unsigned char *data,
                              const int &width, const int &height) {
    processPaintDabs(mDabs, width, height, width, data);
    mDabs.clear();
}

#include "pointhelpers.h"
#include "PixmapEffects/fmt_filters.h"
void BrushEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                           const fmt_filters::image &img,
                                           const qreal &scale) {
    int width = imgPtr.width();
    int height = imgPtr.height();
    QList<BrushStroke*> strokes;
    qsrand(seed);
    int seedAssist = qrand() % 999999;
    if(randomize) {
        seedAssist += relFrame / randStep;
    }
    qsrand(seedAssist);
    for(int i = 0; i < numberStrokes; i++) {
        qreal radius = qRandF(minBrushRadius,
                              maxBrushRadius)*scale;
        QPointF startPos = QPointF(qRandF() * width, qRandF() * height);
        qreal angle;
        if(qRandF() > .5) {
            angle = qRandF(strokeMinDirectionAngle,
                           strokeMaxDirectionAngle);
        } else {
            angle = qRandF(strokeMinDirectionAngle + 180,
                           strokeMaxDirectionAngle + 180);
        }
        qreal length = qRandF(2*radius,
                              strokeMaxLength*scale);
        QLineF line = QLineF(startPos,
                             QPointF(startPos.x() + length,
                                     startPos.y()));
        line.setAngle(angle);
        QPointF endPos = line.p2();
        QPointF point;
        if(line.intersect(QLineF(0., 0., 0., height - 1),
                          &point) == QLineF::BoundedIntersection) {
            endPos = point;
        } else if(line.intersect(QLineF(width - 1, 0.,
                                        width - 1, height - 1),
                                 &point) == QLineF::BoundedIntersection) {
            endPos = point;
        } else if(line.intersect(QLineF(0., 0.,
                                        width - 1, 0.),
                                 &point) == QLineF::BoundedIntersection) {
            endPos = point;
        } else if(line.intersect(QLineF(0., height - 1,
                                        width - 1, height - 1),
                                 &point) == QLineF::BoundedIntersection) {
            endPos = point;
        }
        line = line.normalVector().translated((endPos - startPos)*0.5);
        line.setLength(line.length()*qRandF(-strokeCurvature,
                                            strokeCurvature));
        QPointF ctrls = line.p2();
        if(ctrls.x() >= width) {
            ctrls.setX(width - 2);
        }
        if(ctrls.x() < 0) {
            ctrls.setX(2);
        }
        if(ctrls.y() >= height) {
            ctrls.setY(height - 2);
        }
        if(ctrls.y() < 0) {
            ctrls.setY(2);
        }
        QColor strokeColor = imgPtr.getColor(startPos.x(), startPos.y());
        strokeColor.setHslF(qclamp(strokeColor.hueF() +
                                   qRandF(-0.05, 0.05), 0., 1.),
                            strokeColor.saturationF(),
                            strokeColor.lightnessF(),
                            strokeColor.alphaF());
        BrushStroke *stroke = new BrushStroke(startPos,
                           ctrls,
                           ctrls,
                           endPos,
                           radius,
                           strokeColor);
        strokes << stroke;
        if(!smooth || relFrame % randStep == 0) {
            stroke->prepareToDrawOnImage(imgPtr);
        }
    }

    if(smooth && relFrame % randStep != 0) {
        qsrand(seedAssist + 1);
        qreal weight = qAbs(relFrame % randStep)*1./randStep;
        foreach(BrushStroke *stroke, strokes) {
            qreal radius = qRandF(minBrushRadius,
                                  maxBrushRadius)*scale;
            QPointF startPos = QPointF(qRandF() * width, qRandF() * height);
            qreal angle;
            if(qRandF() > .5) {
                angle = qRandF(strokeMinDirectionAngle,
                               strokeMaxDirectionAngle);
            } else {
                angle = qRandF(strokeMinDirectionAngle + 180,
                               strokeMaxDirectionAngle + 180);
            }
            qreal length = qRandF(2*radius,
                                  strokeMaxLength*scale);
            QLineF line = QLineF(startPos,
                                 QPointF(startPos.x() + length,
                                         startPos.y()));
            line.setAngle(angle);
            QPointF endPos = line.p2();
            QPointF point;
            if(line.intersect(QLineF(0., 0., 0., height - 1),
                              &point) == QLineF::BoundedIntersection) {
                endPos = point;
            } else if(line.intersect(QLineF(width - 1, 0.,
                                            width - 1, height - 1),
                                     &point) == QLineF::BoundedIntersection) {
                endPos = point;
            } else if(line.intersect(QLineF(0., 0.,
                                            width - 1, 0.),
                                     &point) == QLineF::BoundedIntersection) {
                endPos = point;
            } else if(line.intersect(QLineF(0., height - 1,
                                            width - 1, height - 1),
                                     &point) == QLineF::BoundedIntersection) {
                endPos = point;
            }
            line = line.normalVector().translated((endPos - startPos)*0.5);
            line.setLength(line.length()*qRandF(-strokeCurvature,
                                                strokeCurvature));
            QPointF ctrls = line.p2();
            if(ctrls.x() >= width) {
                ctrls.setX(width - 2);
            }
            if(ctrls.x() < 0) {
                ctrls.setX(2);
            }
            if(ctrls.y() >= height) {
                ctrls.setY(height - 2);
            }
            if(ctrls.y() < 0) {
                ctrls.setY(2);
            }
            QColor strokeColor = imgPtr.getColor(startPos.x(), startPos.y());
            strokeColor.setHslF(qclamp(strokeColor.hueF() +
                                       qRandF(-0.05, 0.05), 0., 1.),
                                strokeColor.saturationF(),
                                strokeColor.lightnessF(),
                                strokeColor.alphaF());

            stroke->interpolateWith(startPos, ctrls, ctrls, endPos,
                                    radius, strokeColor, weight);
            stroke->prepareToDrawOnImage(imgPtr);
        }
    }

    unsigned char *data = img.data;
    for(int i = 0; i < width*height*4; i++) {
        data[i] = (unsigned char)0;
    }

    Q_FOREACH(BrushStroke *stroke, strokes) {
        stroke->drawOnImage(data, width, height);
        delete stroke;
    }
}
