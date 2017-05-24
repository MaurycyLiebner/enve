#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H
#include <QDebug>
#include <QtXml/QDomDocument>
#include "fillstrokesettings.h"
#include <QtMath>
class VectorPath;
class Canvas;
class BoxesGroup;
class BoundingBox;
enum CtrlsMode : short;

class TextSvgAttributes {
public:
    TextSvgAttributes();

    TextSvgAttributes &operator*=(const TextSvgAttributes &overwritter);

    void setFontFamily(const QString &family);

    void setFontSize(const int &size);

    void setFontStyle(const QFont::Style &style);

    void setFontWeight(const int &weight);

    void setFontAlignment(const Qt::Alignment &alignment);

private:
    QFont mFont;
    Qt::Alignment mAlignment = Qt::AlignLeft;
};

class FillSvgAttributes {
public:
    FillSvgAttributes();

    FillSvgAttributes &operator*=(const FillSvgAttributes &overwritter);

    void setColor(const Color &val);

    void setColorOpacity(const qreal &opacity);

    void setPaintType(const PaintType &type);

    void setGradient(Gradient *gradient);

    const Color &getColor() const;
    const PaintType &getPaintType() const;
    Gradient *getGradient() const;

    void apply(BoundingBox *box);
protected:
    Color mColor;
    PaintType mPaintType = NOPAINT;
    Gradient *mGradient = NULL;
};

class StrokeSvgAttributes : public FillSvgAttributes {
public:
    StrokeSvgAttributes();



    StrokeSvgAttributes &operator*=(const StrokeSvgAttributes &overwritter);
    const qreal &getLineWidth() const;
    const Qt::PenCapStyle &getCapStyle() const;
    const Qt::PenJoinStyle &getJoinStyle() const;
    const QPainter::CompositionMode &getOutlineCompositionMode() const;

    void setLineWidth(const qreal &val);

    void setCapStyle(const Qt::PenCapStyle &capStyle);

    void setJoinStyle(const Qt::PenJoinStyle &joinStyle);

    void setOutlineCompositionMode(const QPainter::CompositionMode &compMode);

    void apply(BoundingBox *box, const qreal &scale);
protected:
    qreal mLineWidth;
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode = QPainter::CompositionMode_Source;
};

class BoundingBoxSvgAttributes {
public:
    BoundingBoxSvgAttributes();

    virtual ~BoundingBoxSvgAttributes();

    BoundingBoxSvgAttributes &operator*=(const BoundingBoxSvgAttributes &overwritter);

    const Qt::FillRule &getFillRule() const;
    const QMatrix &getRelTransform() const;
    const FillSvgAttributes &getFillAttributes() const;
    const StrokeSvgAttributes &getStrokeAttributes() const;
    const TextSvgAttributes &getTextAttributes() const;

    void loadBoundingBoxAttributes(const QDomElement &element);

    bool hasTransform() const;

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
    SvgPathPoint(QPointF point);

    void guessCtrlsMode();

    void setStartPoint(QPointF startPoint);

    void setEndPoint(QPointF endPoint);

    CtrlsMode getCtrlsMode() const;

    QPointF getPoint() const;

    QPointF getStartPoint() const;

    QPointF getEndPoint() const;

    bool getStartPointEnabled();

    bool getEndPointEnabled();

    void applyTransfromation(const QMatrix &transformation);
private:
    bool mStartPointSet = false;
    QPointF mStartPoint;
    bool mEndPointSet = false;
    QPointF mEndPoint;
    QPointF mPoint;
    CtrlsMode mCtrlsMode;
};

class PathAnimator;
class SinglePathAnimator;
class SvgSeparatePath {
public:
    SvgSeparatePath();
    virtual ~SvgSeparatePath();

    void apply(SinglePathAnimator *path);

    void closePath();

    void moveTo(QPointF e);

    void cubicTo(QPointF c1, QPointF c2, QPointF e);

    void lineTo(QPointF e);

    void quadTo(QPointF c, QPointF e);

    void applyTransfromation(const QMatrix &transformation);

    void pathArc(qreal rx,
                 qreal ry,
                 qreal x_axis_rotation,
                 int large_arc_flag,
                 int sweep_flag,
                 qreal x,
                 qreal y,
                 qreal curx, qreal cury);
private:
    void pathArcSegment(qreal xc, qreal yc,
                        qreal th0, qreal th1,
                        qreal rx, qreal ry, qreal xAxisRotation);

    void addPoint(SvgPathPoint *point);
    SvgPathPoint *mFirstPoint = NULL;
    SvgPathPoint *mLastPoint = NULL;
    QList<SvgPathPoint*> mPoints;
    bool mClosedPath = false;
};


class VectorPathSvgAttributes : public BoundingBoxSvgAttributes {
public:
    VectorPathSvgAttributes() {}
    ~VectorPathSvgAttributes() {
        Q_FOREACH(SvgSeparatePath *path, mSvgSeparatePaths) {
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
