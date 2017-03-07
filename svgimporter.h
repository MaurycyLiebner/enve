#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H
#include <QDebug>
#include <QtXml/QDomDocument>
#include "fillstrokesettings.h"
#include <QtMath>
class VectorPath;
class Canvas;
class BoxesGroup;

class TextSvgAttributes {
public:
    TextSvgAttributes() {}

    TextSvgAttributes &operator*=(const TextSvgAttributes &overwritter)
    {
//        if(overwritter.wasColorAssigned()) {
//            mColor = overwritter.getColor();
//        }
        return *this;
    }

    void setFontFamily(const QString &family) {
        mFont.setFamily(family);
    }

    void setFontSize(const int &size) {
        mFont.setPixelSize(size);
    }

    void setFontStyle(const QFont::Style &style) {
        mFont.setStyle(style);
    }

    void setFontWeight(const int &weight) {
        mFont.setWeight(weight);
    }

    void setFontAlignment(const Qt::Alignment &alignment) {
        mAlignment = alignment;
    }

private:
    QFont mFont;
    Qt::Alignment mAlignment = Qt::AlignLeft;
};

class FillSvgAttributes {
public:
    FillSvgAttributes() {}

    FillSvgAttributes &operator*=(const FillSvgAttributes &overwritter) {
        setColor(overwritter.getColor());
        setPaintType(overwritter.getPaintType());
        setGradient(overwritter.getGradient());
        return *this;
    }

    void setColor(const Color &val) {
        qreal opacity = val.gl_a;
        mColor = val;
        mColor.setGLColorA(opacity);
        setPaintType(FLATPAINT);
    }

    void setColorOpacity(const qreal &opacity) {
        mColor.setGLColorA(opacity);
    }

    void setPaintType(const PaintType &type) {
        mPaintType = type;
    }

    void setGradient(Gradient *gradient) {
        mGradient = gradient;
        setPaintType(GRADIENTPAINT);
    }

    const Color &getColor() const { return mColor; }
    const PaintType &getPaintType() const { return mPaintType; }
    Gradient *getGradient() const { return mGradient; }

    void apply(BoundingBox *box);
protected:
    Color mColor;
    PaintType mPaintType = NOPAINT;
    Gradient *mGradient = NULL;
};

class StrokeSvgAttributes : public FillSvgAttributes {
public:
    StrokeSvgAttributes() {}



    StrokeSvgAttributes &operator*=(const StrokeSvgAttributes &overwritter) {
        setColor(overwritter.getColor());
        setPaintType(overwritter.getPaintType());
        setGradient(overwritter.getGradient());

        setLineWidth(overwritter.getLineWidth());
        setCapStyle(overwritter.getCapStyle());
        setJoinStyle(overwritter.getJoinStyle());
        setOutlineCompositionMode(overwritter.getOutlineCompositionMode());
        return *this;
    }
    const qreal &getLineWidth() const { return mLineWidth; }
    const Qt::PenCapStyle &getCapStyle() const { return mCapStyle; }
    const Qt::PenJoinStyle &getJoinStyle() const { return mJoinStyle; }
    const QPainter::CompositionMode &getOutlineCompositionMode() const { return mOutlineCompositionMode; }

    void setLineWidth(const qreal &val) {
        mLineWidth = val;
    }

    void setCapStyle(const Qt::PenCapStyle &capStyle) {
        mCapStyle = capStyle;
    }

    void setJoinStyle(const Qt::PenJoinStyle &joinStyle) {
        mJoinStyle = joinStyle;
    }

    void setOutlineCompositionMode(const QPainter::CompositionMode &compMode) {
        mOutlineCompositionMode = compMode;
    }

    void apply(BoundingBox *box, const qreal &scale);
protected:
    qreal mLineWidth;
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode = QPainter::CompositionMode_Source;
};

class BoundingBoxSvgAttributes {
public:
    BoundingBoxSvgAttributes() {}

    virtual ~BoundingBoxSvgAttributes() {}

    BoundingBoxSvgAttributes &operator*=(const BoundingBoxSvgAttributes &overwritter)
    {
        mRelTransform = overwritter.getRelTransform();

        mFillAttributes *= overwritter.getFillAttributes();
        mStrokeAttributes *= overwritter.getStrokeAttributes();
        mTextAttributes *= overwritter.getTextAttributes();
        mFillRule = overwritter.getFillRule();

        return *this;
    }

    const Qt::FillRule &getFillRule() const { return mFillRule; }
    const QMatrix &getRelTransform() const { return mRelTransform; }
    const FillSvgAttributes &getFillAttributes() const { return mFillAttributes; }
    const StrokeSvgAttributes &getStrokeAttributes() const { return mStrokeAttributes; }
    const TextSvgAttributes &getTextAttributes() const { return mTextAttributes; }

    void loadBoundingBoxAttributes(const QDomElement &element);

    bool hasTransform() const {
        return !(isZero(mRelTransform.dx()) &&
                 isZero(mRelTransform.dy()) &&
                 isZero(mRelTransform.m11() - 1.) &&
                 isZero(mRelTransform.m22() - 1) &&
                 isZero(mRelTransform.m12()) &&
                 isZero(mRelTransform.m21())); /*&&
                 isZero(mDx) && isZero(mDy) &&
                 isZero(mScaleX - 1.) && isZero(mScaleY - 1.) &&
                 isZero(mShearX) && isZero(mShearY) &&
                 isZero(mRot));*/
    }

    void applySingleTransformations(BoundingBox *box);

    void apply(BoundingBox *box);
protected:
    qreal mDx = 0.;
    qreal mDy = 0.;
    qreal mScaleX = 1.;
    qreal mScaleY = 1.;
    qreal mShearX = 0.;
    qreal mShearY = 0.;
    qreal mRot = 0.;

    QString mId;
    QMatrix mRelTransform;
    FillSvgAttributes mFillAttributes;
    StrokeSvgAttributes mStrokeAttributes;
    TextSvgAttributes mTextAttributes;
    qreal mOpacity = 100.;
    Qt::FillRule mFillRule = Qt::OddEvenFill;
};

class SvgPathPoint {
public:
    SvgPathPoint(QPointF point) {
        mPoint = point;
    }

    void guessCtrlsMode() {
        QPointF startPointRel = mStartPoint - mPoint;
        QPointF endPointRel = mEndPoint - mPoint;
        if(isZero1Dec(pointToLen(startPointRel) - pointToLen(endPointRel)) ) {
            mCtrlsMode = CTRLS_SYMMETRIC;
        } else {
            qreal angle = QLineF(mStartPoint, mPoint).angleTo(QLineF(mPoint, mEndPoint));
            while(angle > 90.) angle -= 180;
            if(isZero1Dec(angle)) {
                mCtrlsMode = CTRLS_SMOOTH;
            }
        }
    }

    void setStartPoint(QPointF startPoint) {
        mStartPoint = startPoint;
        if(isZero1Dec(pointToLen(mStartPoint - mPoint))) return;
        mStartPointSet = true;
        if(mEndPointSet) guessCtrlsMode();
    }

    void setEndPoint(QPointF endPoint) {
        mEndPoint = endPoint;
        if(isZero1Dec(pointToLen(mEndPoint - mPoint))) return;
        mEndPointSet = true;
        if(mStartPointSet) guessCtrlsMode();
    }

    CtrlsMode getCtrlsMode() const {
        return mCtrlsMode;
    }

    QPointF getPoint() const {
        return mPoint;
    }

    QPointF getStartPoint() const {
        return mStartPoint;
    }

    QPointF getEndPoint() const {
        return mEndPoint;
    }

    bool getStartPointEnabled() {
        return mStartPointSet;
    }

    bool getEndPointEnabled() {
        return mEndPointSet;
    }

    void applyTransfromation(const QMatrix &transformation) {
        mPoint = transformation.map(mPoint);
        mEndPoint = transformation.map(mEndPoint);
        mStartPoint = transformation.map(mStartPoint);
    }
private:
    bool mStartPointSet = false;
    QPointF mStartPoint;
    bool mEndPointSet = false;
    QPointF mEndPoint;
    QPointF mPoint;
    CtrlsMode mCtrlsMode = CTRLS_CORNER;
};

class PathAnimator;
class SinglePathAnimator;
class SvgSeparatePath {
public:
    SvgSeparatePath() {}
    virtual ~SvgSeparatePath() {
        foreach(SvgPathPoint *point, mPoints) {
            delete point;
        }
    }

    void apply(SinglePathAnimator *path);

    void closePath() {
        if(mLastPoint->getStartPointEnabled() &&
           pointToLen(mLastPoint->getPoint() - mFirstPoint->getPoint()) < 0.1) {
            mFirstPoint->setStartPoint(mLastPoint->getStartPoint());
            delete mPoints.takeLast();
            mLastPoint = mPoints.last();
        }

        mClosedPath = true;
    }

    void moveTo(QPointF e) {
        mFirstPoint = new SvgPathPoint(e);
        mLastPoint = mFirstPoint;
        addPoint(mFirstPoint);
    }

    void cubicTo(QPointF c1, QPointF c2, QPointF e) {
        mLastPoint->setEndPoint(c1);
        mLastPoint = new SvgPathPoint(e);
        mLastPoint->setStartPoint(c2);
        addPoint(mLastPoint);
    }

    void lineTo(QPointF e) {
        mLastPoint = new SvgPathPoint(e);
        addPoint(mLastPoint);
    }

    void quadTo(QPointF c, QPointF e) {
        QPointF prev = mLastPoint->getPoint();
        QPointF c1((prev.x() + 2*c.x()) / 3, (prev.y() + 2*c.y()) / 3);
        QPointF c2((e.x() + 2*c.x()) / 3, (e.y() + 2*c.y()) / 3);
        cubicTo(c1, c2, e);
    }

    void applyTransfromation(const QMatrix &transformation) {
        foreach(SvgPathPoint *point, mPoints) {
            point->applyTransfromation(transformation);
        }
    }

    void pathArc(qreal rx,
                 qreal ry,
                 qreal x_axis_rotation,
                 int large_arc_flag,
                 int sweep_flag,
                 qreal x,
                 qreal y,
                 qreal curx, qreal cury) {
        qreal sin_th, cos_th;
        qreal a00, a01, a10, a11;
        qreal x0, y0, x1, y1, xc, yc;
        qreal d, sfactor, sfactor_sq;
        qreal th0, th1, th_arc;
        int i, n_segs;
        qreal dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;

        rx = qAbs(rx);
        ry = qAbs(ry);

        sin_th = qSin(x_axis_rotation * (M_PI / 180.0));
        cos_th = qCos(x_axis_rotation * (M_PI / 180.0));

        dx = (curx - x) / 2.0;
        dy = (cury - y) / 2.0;
        dx1 =  cos_th * dx + sin_th * dy;
        dy1 = -sin_th * dx + cos_th * dy;
        Pr1 = rx * rx;
        Pr2 = ry * ry;
        Px = dx1 * dx1;
        Py = dy1 * dy1;
        /* Spec : check if radii are large enough */
        check = Px / Pr1 + Py / Pr2;
        if (check > 1) {
            rx = rx * qSqrt(check);
            ry = ry * qSqrt(check);
        }

        a00 =  cos_th / rx;
        a01 =  sin_th / rx;
        a10 = -sin_th / ry;
        a11 =  cos_th / ry;
        x0 = a00 * curx + a01 * cury;
        y0 = a10 * curx + a11 * cury;
        x1 = a00 * x + a01 * y;
        y1 = a10 * x + a11 * y;
        /* (x0, y0) is current point in transformed coordinate space.
           (x1, y1) is new point in transformed coordinate space.

           The arc fits a unit-radius circle in this space.
        */
        d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
        sfactor_sq = 1.0 / d - 0.25;
        if (sfactor_sq < 0) sfactor_sq = 0;
        sfactor = qSqrt(sfactor_sq);
        if (sweep_flag == large_arc_flag) sfactor = -sfactor;
        xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
        yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
        /* (xc, yc) is center of the circle. */

        th0 = qAtan2(y0 - yc, x0 - xc);
        th1 = qAtan2(y1 - yc, x1 - xc);

        th_arc = th1 - th0;
        if (th_arc < 0 && sweep_flag)
            th_arc += 2 * M_PI;
        else if (th_arc > 0 && !sweep_flag)
            th_arc -= 2 * M_PI;

        n_segs = qCeil(qAbs(th_arc / (M_PI * 0.5 + 0.001)));

        for (i = 0; i < n_segs; i++) {
            pathArcSegment(xc, yc,
                           th0 + i * th_arc / n_segs,
                           th0 + (i + 1) * th_arc / n_segs,
                           rx, ry, x_axis_rotation);
        }
    }
private:
    void pathArcSegment(qreal xc, qreal yc,
                        qreal th0, qreal th1,
                        qreal rx, qreal ry, qreal xAxisRotation)
    {
        qreal sinTh, cosTh;
        qreal a00, a01, a10, a11;
        qreal x1, y1, x2, y2, x3, y3;
        qreal t;
        qreal thHalf;

        sinTh = qSin(xAxisRotation * (M_PI / 180.0));
        cosTh = qCos(xAxisRotation * (M_PI / 180.0));

        a00 =  cosTh * rx;
        a01 = -sinTh * ry;
        a10 =  sinTh * rx;
        a11 =  cosTh * ry;

        thHalf = 0.5 * (th1 - th0);
        t = (8.0 / 3.0) * qSin(thHalf * 0.5) * qSin(thHalf * 0.5) / qSin(thHalf);
        x1 = xc + qCos(th0) - t * qSin(th0);
        y1 = yc + qSin(th0) + t * qCos(th0);
        x3 = xc + qCos(th1);
        y3 = yc + qSin(th1);
        x2 = x3 + t * qSin(th1);
        y2 = y3 - t * qCos(th1);

        cubicTo(QPointF(a00 * x1 + a01 * y1, a10 * x1 + a11 * y1),
                QPointF(a00 * x2 + a01 * y2, a10 * x2 + a11 * y2),
                QPointF(a00 * x3 + a01 * y3, a10 * x3 + a11 * y3));
    }

    void addPoint(SvgPathPoint *point) {
        mPoints << point;
    }
    SvgPathPoint *mFirstPoint = NULL;
    SvgPathPoint *mLastPoint = NULL;
    QList<SvgPathPoint*> mPoints;
    bool mClosedPath = false;
};


class VectorPathSvgAttributes : public BoundingBoxSvgAttributes {
public:
    VectorPathSvgAttributes() {}
    ~VectorPathSvgAttributes() {
        foreach(SvgSeparatePath *path, mSvgSeparatePaths) {
            delete path;
        }
    }

    SvgSeparatePath *newSeparatePath() {
        SvgSeparatePath *lastPath = new SvgSeparatePath();
        mSvgSeparatePaths << lastPath;
        return lastPath;
    }
    void apply(VectorPath *path);
protected:
    QList<SvgSeparatePath*> mSvgSeparatePaths;
};


extern void loadBoxesGroup(const QDomElement &groupElement, BoxesGroup *parentGroup, BoundingBoxSvgAttributes *attributes);
extern bool parsePathDataFast(const QString &dataStr, VectorPathSvgAttributes *attributes);
extern void loadVectorPath(const QDomElement &pathElement, BoxesGroup *parentGroup, VectorPathSvgAttributes *attributes);
extern void loadElement(const QDomElement &element, BoxesGroup *parentGroup, BoundingBoxSvgAttributes *parentGroupAttributes);
extern void loadSVGFile(const QString &filename, Canvas *canvas);

extern QMatrix getMatrixFromString(const QString &matrixStr);
extern bool getColorFromString(const QString &colorStr, Color *color);

/*
#include <QStringRef>
#include <QPainterPath>
#include <QXmlStreamAttributes>

class BoxesGroup;

class VectorPath;

extern QVector<qreal> parsePercentageList(const QChar *&str);
extern bool resolveColor(const QStringRef &colorStr, QColor &color, QSvgHandler *handler);
extern bool parsePathDataFast(const QStringRef &dataStr, VectorPath *path);
extern VectorPath *createPathNode(BoxesGroup *parent,
                                const QXmlStreamAttributes &attributes);
extern QColor parseColor(const QSvgAttributes &attributes,
                       QSvgHandler *handler);
extern bool constructColor(const QStringRef &colorStr, const QStringRef &opacity,
                           QColor &color, QSvgHandler *handler);
*/
#endif // SVGIMPORTER_H
