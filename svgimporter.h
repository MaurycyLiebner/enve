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

    const QFont &getFont() const { return mFont; }
private:
    Qt::Alignment mAlignment = Qt::AlignLeft;
    QFont mFont;
};

class GradientsSvgCollection {
public:
    GradientsSvgCollection() {}

    void addGradient(Gradient* gradient,
                     const QString& name) {
        mGradientNames << name;
        mGradients << gradient;
    }

    Gradient* getGradientWithId(QString id) {
        if(id.isEmpty()) return nullptr;
        if(id.at(0) == '#') id.remove(0, 1);

        for(int i = 0; i < mGradientNames.count(); i++) {
            if(mGradientNames.at(i) == id) {
                return mGradients.at(i);
            }
        }
        return nullptr;
    }
private:
    QList<QString> mGradientNames;
    QList<Gradient*> mGradients;
};

class FillSvgAttributes {
public:
    FillSvgAttributes();

    FillSvgAttributes &operator*=(const FillSvgAttributes &overwritter);

    void setColor(const QColor &val);

    void setColorOpacity(const qreal &opacity);

    void setPaintType(const PaintType &type);

    void setGradient(Gradient *gradient);

    const QColor &getColor() const;
    const PaintType &getPaintType() const;
    Gradient *getGradient() const;

    void apply(BoundingBox *box);
    void apply(BoundingBox *box, const bool &isFill);
protected:
    QColor mColor = QColor(0, 0, 0);
    PaintType mPaintType = FLATPAINT;//NOPAINT;
    Gradient *mGradient = nullptr;
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
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
    qreal mLineWidth = 0.;
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
    void setFillAttribute(const QString &value);
    void setStrokeAttribute(const QString &value);
protected:
    Qt::FillRule mFillRule = Qt::OddEvenFill;

    qreal mDx = 0.;
    qreal mDy = 0.;
    qreal mScaleX = 1.;
    qreal mScaleY = 1.;
    qreal mShearX = 0.;
    qreal mShearY = 0.;
    qreal mRot = 0.;

    qreal mOpacity = 100.;

    QMatrix mRelTransform;

    QString mId;

    FillSvgAttributes mFillAttributes;
    StrokeSvgAttributes mStrokeAttributes;
    TextSvgAttributes mTextAttributes;
};

class SvgNodePoint : public StdSelfRef {
public:
    SvgNodePoint(QPointF point);

    void guessCtrlsMode();

    void setStartPoint(const QPointF &startPoint);

    void setEndPoint(const QPointF &endPoint);

    CtrlsMode getCtrlsMode() const;

    QPointF getPoint() const;

    QPointF getStartPoint() const;

    QPointF getEndPoint() const;

    bool getStartPointEnabled() const;

    bool getEndPointEnabled() const;

    void applyTransfromation(const QMatrix &transformation);
private:
    bool mStartPointSet = false;
    bool mEndPointSet = false;
    CtrlsMode mCtrlsMode;
    QPointF mStartPoint;
    QPointF mEndPoint;
    QPointF mPoint;
};

class PathAnimator;
class VectorPathAnimator;
class SvgSeparatePath : public StdSelfRef {
    friend class StdSelfRef;
public:
    void apply(VectorPathAnimator *path);

    void closePath();

    void moveTo(const QPointF &e);

    void cubicTo(const QPointF &c1, const QPointF &c2, const QPointF &e);

    void lineTo(const QPointF &e);

    void quadTo(const QPointF &c, const QPointF &e);

    void applyTransfromation(const QMatrix &transformation);

    void pathArc(qreal rx,
                 qreal ry,
                 qreal x_axis_rotation,
                 int large_arc_flag,
                 int sweep_flag,
                 qreal x,
                 qreal y,
                 qreal curx, qreal cury);
protected:
    SvgSeparatePath() {}
private:
    void pathArcSegment(qreal xc, qreal yc,
                        qreal th0, qreal th1,
                        qreal rx, qreal ry, qreal xAxisRotation);

    void addPoint(const SvgNodePointSPtr& point);

    bool mClosedPath = false;
    SvgNodePointPtr mFirstPoint;
    SvgNodePointPtr mLastPoint;
    QList<SvgNodePointSPtr> mPoints;
};


class VectorPathSvgAttributes : public BoundingBoxSvgAttributes {
public:
    VectorPathSvgAttributes() {}

    SvgSeparatePath *newSeparatePath() {
        SvgSeparatePathSPtr lastPath = SPtrCreate(SvgSeparatePath)();
        mSvgSeparatePaths << lastPath;
        return lastPath.get();
    }
    void apply(VectorPath *path);
protected:
    QList<SvgSeparatePathSPtr> mSvgSeparatePaths;
};


extern void loadElement(const QDomElement &element, BoxesGroup *parentGroup,
                        BoundingBoxSvgAttributes *parentGroupAttributes);
extern BoxesGroupQSPtr loadSVGFile(const QString &filename);
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
