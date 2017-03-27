#include "brusheffect.h"
bool BrushStroke::mBrushPixmapsLoaded = false;

BrushEffect::BrushEffect(qreal numberStrokes,
                         qreal brushMinRadius, qreal brushMaxRadius,
                         qreal strokeMaxLength,
                         qreal strokeMinDirectionAngle,
                         qreal strokeMaxDirectionAngle,
                         qreal strokeCurvature)
{
    BrushStroke::loadStrokePixmaps();
    prp_setName("brush");

    mNumberStrokes.qra_setCurrentValue(numberStrokes);
    mNumberStrokes.prp_setName("number strokes");
    mNumberStrokes.blockPointer();
    ca_addChildAnimator(&mNumberStrokes);

    mMinBrushRadius.qra_setCurrentValue(brushMinRadius);
    mMinBrushRadius.prp_setName("min radius");
    mMinBrushRadius.blockPointer();
    ca_addChildAnimator(&mMinBrushRadius);

    mMaxBrushRadius.qra_setCurrentValue(brushMaxRadius);
    mMaxBrushRadius.prp_setName("max radius");
    mMaxBrushRadius.blockPointer();
    ca_addChildAnimator(&mMaxBrushRadius);

    mStrokeMaxLength.qra_setCurrentValue(strokeMaxLength);
    mStrokeMaxLength.prp_setName("max stroke length");
    mStrokeMaxLength.blockPointer();
    ca_addChildAnimator(&mStrokeMaxLength);

    mStrokeMinDirectionAngle.qra_setCurrentValue(strokeMinDirectionAngle);
    mStrokeMinDirectionAngle.prp_setName("min stroke angle");
    mStrokeMinDirectionAngle.blockPointer();
    ca_addChildAnimator(&mStrokeMinDirectionAngle);

    mStrokeMaxDirectionAngle.qra_setCurrentValue(strokeMaxDirectionAngle);
    mStrokeMaxDirectionAngle.prp_setName("max stroke angle");
    mStrokeMaxDirectionAngle.blockPointer();
    ca_addChildAnimator(&mStrokeMaxDirectionAngle);

    mStrokeCurvature.qra_setCurrentValue(strokeCurvature);
    mStrokeCurvature.prp_setName("curvature");
    mStrokeCurvature.blockPointer();
    ca_addChildAnimator(&mStrokeCurvature);
}

void BrushEffect::apply(BoundingBox *target, QImage *imgPtr,
                        const fmt_filters::image &img,
                        qreal scale) {
    int width = imgPtr->width();
    int height = imgPtr->height();
    QList<BrushStroke*> strokes;
    for(int i = 0; i < mNumberStrokes.qra_getCurrentValue(); i++) {
        qreal radius = qRandF(mMinBrushRadius.qra_getCurrentValue(),
                              mMaxBrushRadius.qra_getCurrentValue())*scale;
        QPointF startPos = QPointF(qrand() % width, qrand() % height);
        qreal angle;
        if(qrand() % 2 == 1) {
            angle = qRandF(mStrokeMinDirectionAngle.qra_getCurrentValue(),
                           mStrokeMaxDirectionAngle.qra_getCurrentValue());
        } else {
            angle = qRandF(mStrokeMinDirectionAngle.qra_getCurrentValue() + 180,
                           mStrokeMaxDirectionAngle.qra_getCurrentValue() + 180);
        }
        qreal length = qRandF(2*radius,
                              mStrokeMaxLength.qra_getCurrentValue()*scale);
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
        line.setLength(line.length()*qRandF(-mStrokeCurvature.qra_getCurrentValue(),
                                            mStrokeCurvature.qra_getCurrentValue()));
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
        QColor strokeColor = imgPtr->pixelColor(startPos.toPoint());
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
        stroke->prepareToDrawOnImage(imgPtr);
    }

    imgPtr->fill(Qt::transparent);

    foreach(BrushStroke *stroke, strokes) {
        stroke->drawOnImage(imgPtr);
        delete stroke;
    }
}

qreal BrushEffect::getMargin() {
    return mMaxBrushRadius.qra_getCurrentValue();
}

BrushStroke::BrushStroke(QPointF startPos, QPointF startCtrlPos,
                         QPointF endCtrlPos, QPointF endPos,
                         qreal radius, QColor color) {
    mStartPos = startPos;
    mStartCtrlPos = startCtrlPos;
    mEndCtrlPos = endCtrlPos;
    mEndPos = endPos;
    mRadius = radius;
    mColor = color;

    mStrokePath = QPainterPath(mStartPos);
    mStrokePath.cubicTo(mStartCtrlPos, mEndCtrlPos, mEndPos);

    QPainterPathStroker stroker;
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setWidth(2*mRadius);
    mWholeStrokePath = stroker.createStroke(mStrokePath);
    mBoundingRect = mWholeStrokePath.boundingRect();
}

void BrushStroke::drawOnImage(QImage *img) const {
    QImage strokeImg = QImage(mBoundingRect.size().toSize(),
                              QImage::Format_ARGB32_Premultiplied);
    strokeImg.fill(Qt::transparent);
    QPainter pStroke(&strokeImg);
    pStroke.setRenderHint(QPainter::SmoothPixmapTransform);
    pStroke.setCompositionMode(QPainter::CompositionMode_Source);

    pStroke.drawPixmapFragments(mFragments, mNDabs, mTexPix);

    pStroke.end();


    QPainter p(img);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    p.drawImage(mBoundingRect.topLeft(), strokeImg);

    p.end();
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

QList<QPixmap*> BrushStroke::mStrokeTexPixmaps;
QPixmap *BrushStroke::mEndPix;

void BrushStroke::loadStrokePixmaps() {
    if(mBrushPixmapsLoaded) return;
    mEndPix = new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/end.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/1.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/2.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/3.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/4.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/5.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/6.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/7.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/8.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/9.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/10.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/11.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/12.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/13.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/14.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/15.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/16.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/17.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/18.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/19.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/20.png");
    mStrokeTexPixmaps << new QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/21.png");
    mBrushPixmapsLoaded = true;
}

#include <QDebug>
void BrushStroke::prepareToDrawOnImage(QImage *img)
{
    mTexPix = mStrokeTexPixmaps.at(qrand() % 21)->
            scaledToHeight(mRadius*2, Qt::SmoothTransformation);
    mStrokeTexHeight = mTexPix.height();
    mStrokeTexWidth = mTexPix.width();
    mMaxTexDabs = mStrokeTexWidth*0.5;
    mMaxStrokeDabs = mStrokePath.length()*0.5;
    mNDabs = qMin(mMaxTexDabs, mMaxStrokeDabs);

    int dabsWidth = mNDabs*2;
    QLinearGradient gradient = QLinearGradient(0., 0., dabsWidth, 0.);
    //gradient.setColorAt(0., mColor);
    for(qreal xTex = 0; xTex < dabsWidth; xTex += mRadius) {
        QColor colAtPix = img->pixelColor(
                    mStrokePath.pointAtPercent(
                        mStrokePath.percentAtLength(xTex)).toPoint());
        if(isTooDifferent(colAtPix, mColor)) {
            mMaxStrokeDabs = xTex*0.5;
            mNDabs = mMaxStrokeDabs;
            dabsWidth = xTex;
            break;
        }
        QColor col = colorMix(colAtPix, mColor);
        gradient.setColorAt((xTex /*+ mRadius*/)/dabsWidth, col);
    }

    QPainter pTex(&mTexPix);
    pTex.setCompositionMode(QPainter::CompositionMode_SourceIn);
    if(mMaxTexDabs > mMaxStrokeDabs) {
        QPixmap endPix = mEndPix->scaledToHeight(mRadius*2);
        pTex.drawPixmap(dabsWidth - endPix.width() + 2, 0,
                        endPix );
    }

    pTex.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pTex.fillRect(mTexPix.rect(), gradient);

    pTex.end();


    mFragments = new QPainter::PixmapFragment[mNDabs];
    qreal lastRadAngle = 0;
    for(int i = 0; i < mNDabs; i++) {
        qreal perc = mStrokePath.percentAtLength(i*2);
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
        qreal paintWidth = ((i == 0) ? 3. : 3. +
                            sin(qAbs(lastRadAngle - radAngle)*0.5)*mRadius);
        lastRadAngle = radAngle;
        mFragments[i] = QPainter::PixmapFragment::create(
                    mStrokePath.pointAtPercent(perc) - mBoundingRect.topLeft(),
                    QRectF(i*2, 0, paintWidth, mStrokeTexHeight),
                    1., 1.,
                    radAngle * 180 / M_PI );
    }
}
