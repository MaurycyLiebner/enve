#include "pixmapeffect.h"

PixmapEffect::PixmapEffect() : ComplexAnimator()
{

}

BlurEffect::BlurEffect(qreal radius) {
    mBlurRadius.setCurrentValue(radius);
    setName("blur");
    mBlurRadius.setName("radius");
    mBlurRadius.blockPointer();
    addChildAnimator(&mBlurRadius);
}

void BlurEffect::apply(QImage *imgPtr,
                       const fmt_filters::image &img, qreal scale) {
    Q_UNUSED(imgPtr);
    qreal radius = mBlurRadius.getCurrentValue()*scale;
    fmt_filters::blur(img, radius, radius*0.3333);
}

qreal BlurEffect::getMargin()
{
    return mBlurRadius.getCurrentValue();
}

BrushEffect::BrushEffect(qreal brushMinRadius, qreal brushMaxRadius,
                         qreal strokeMinLength, qreal strokeMaxLength,
                         qreal strokeMinDirectionAngle, qreal strokeMaxDirectionAngle,
                         qreal strokeCurvature)
{
    setName("brush");

    mMinBrushRadius.setCurrentValue(brushMinRadius);
    mMinBrushRadius.setName("min radius");
    mMinBrushRadius.blockPointer();
    addChildAnimator(&mMinBrushRadius);

    mMaxBrushRadius.setCurrentValue(brushMaxRadius);
    mMaxBrushRadius.setName("max radius");
    mMaxBrushRadius.blockPointer();
    addChildAnimator(&mMaxBrushRadius);

    mStrokeMinLength.setCurrentValue(strokeMinLength);
    mStrokeMinLength.setName("min stroke length");
    mStrokeMinLength.blockPointer();
    addChildAnimator(&mStrokeMinLength);

    mStrokeMaxLength.setCurrentValue(strokeMaxLength);
    mStrokeMaxLength.setName("max stroke length");
    mStrokeMaxLength.blockPointer();
    addChildAnimator(&mStrokeMaxLength);

    mStrokeMinDirectionAngle.setCurrentValue(strokeMinDirectionAngle);
    mStrokeMinDirectionAngle.setName("min stroke angle");
    mStrokeMinDirectionAngle.blockPointer();
    addChildAnimator(&mStrokeMinDirectionAngle);

    mStrokeMaxDirectionAngle.setCurrentValue(strokeMaxDirectionAngle);
    mStrokeMaxDirectionAngle.setName("max stroke angle");
    mStrokeMaxDirectionAngle.blockPointer();
    addChildAnimator(&mStrokeMaxDirectionAngle);

    mStrokeCurvature.setCurrentValue(0.5);//strokeCurvature);
    mStrokeCurvature.setName("curvature");
    mStrokeCurvature.blockPointer();
    addChildAnimator(&mStrokeCurvature);
}

void BrushEffect::apply(QImage *imgPtr, const fmt_filters::image &img, qreal scale) {
    int width = imgPtr->width();
    int height = imgPtr->height();
    QList<BrushStroke*> strokes;
    for(int i = 0; i < 300; i++) {
        QPointF startPos = QPointF(qrand() % width, qrand() % height);
        QPointF endPos = QPointF(qrand() % width, qrand() % height);
        QLineF line = QLineF(startPos, endPos).normalVector().translated((endPos - startPos)*0.5);
        line.setLength(line.length()*mStrokeCurvature.getCurrentValue());
        QPointF ctrls = line.p2();
        if(ctrls.x() > width) {
            ctrls.setX(width - 2);
        }
        if(ctrls.x() < 0) {
            ctrls.setX(2);
        }
        if(ctrls.y() > height) {
            ctrls.setY(height - 2);
        }
        if(ctrls.y() < 0) {
            ctrls.setY(2);
        }
        QColor strokeColor = imgPtr->pixelColor(startPos.toPoint());
        strokeColor.setHslF(qclamp(strokeColor.hueF() + 0.01*(qrand() % 10 - 5), 0., 1.),
                            strokeColor.saturationF(),
                            strokeColor.lightnessF(),
                            strokeColor.alphaF());
        BrushStroke *stroke = new BrushStroke(startPos,
                           ctrls,
                           ctrls,
                           endPos,
                           mMinBrushRadius.getCurrentValue(),
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

qreal BrushEffect::getMargin()
{
    return mMaxBrushRadius.getCurrentValue();
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
    QImage strokeImg = QImage(mBoundingRect.size().toSize(), QImage::Format_ARGB32_Premultiplied);
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

void BrushStroke::prepareToDrawOnImage(QImage *img)
{
    mTexPix = QPixmap("/home/ailuropoda/Downloads/036-distressed-halfton-brush-strokes/PNGs/1.png").scaledToHeight(mRadius*2, Qt::SmoothTransformation);
    mStrokeTexHeight = mTexPix.height();
    mStrokeTexWidth = mTexPix.width();
    mMaxTexDabs = mStrokeTexWidth*0.5;
    mMaxStrokeDabs = mStrokePath.length()*0.5;
    mNDabs = qMin(mMaxTexDabs, mMaxStrokeDabs);

    int dabsWidth = mNDabs*2;
    QPainter pTex(&mTexPix);
    QLinearGradient gradient = QLinearGradient(0., 0., dabsWidth, 0.);
    //gradient.setColorAt(0., mColor);
    for(qreal xTex = 0; xTex < dabsWidth; xTex += mRadius) {
        QColor col = colorMix(img->pixelColor(mStrokePath.pointAtPercent(mStrokePath.percentAtLength(xTex)).toPoint() ), mColor);
        gradient.setColorAt((xTex /*+ mRadius*/)/dabsWidth, col);
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
        qreal paintWidth = ((i == 0) ? 3. : 3. + sin(qAbs(lastRadAngle - radAngle)*0.5)*mRadius);
        lastRadAngle = radAngle;
        mFragments[i] = QPainter::PixmapFragment::create(
                    mStrokePath.pointAtPercent(perc) - mBoundingRect.topLeft(),
                    QRectF(i*2, 0, paintWidth, mStrokeTexHeight),
                    1., 1.,
                    radAngle * 180 / M_PI );
    }
}
