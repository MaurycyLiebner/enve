#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H
#include <QDebug>
#include <QtXml/QDomDocument>
#include "fillstrokesettings.h"

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
private:
};

class FillSvgAttributes {
public:
    FillSvgAttributes() {}

    FillSvgAttributes &operator*=(const FillSvgAttributes &overwritter)
    {
        if(overwritter.wasColorAssigned()) {
            mColor = overwritter.getColor();
        }
        if(overwritter.wasPaintTypeAssigned()) {
            mPaintType = overwritter.getPaintType();
        }
        if(overwritter.wasGradientAssigned()) {
            mGradient = overwritter.getGradient();
        }
        return *this;
    }

    void setColor(const Color &val) {
        mColor = val;
        mColorAssigned = true;
        setPaintType(FLATPAINT);
    }

    void setPaintType(const PaintType &type) {
        mPaintType = type;
        mPaintTypeAssigned = true;
    }

    void setGradient(Gradient *gradient) {
        mGradient = gradient;
        mGradientAssigned = true;
        setPaintType(GRADIENTPAINT);
    }

    bool wasColorAssigned() const { return mColorAssigned; }
    bool wasPaintTypeAssigned() const { return mPaintTypeAssigned; }
    bool wasGradientAssigned() const { return mGradientAssigned; }

    const Color &getColor() const { return mColor; }
    const PaintType &getPaintType() const { return mPaintType; }
    Gradient *getGradient() const { return mGradient; }

    void apply(BoundingBox *box);
protected:
    bool mColorAssigned = false;
    Color mColor;
    bool mPaintTypeAssigned = false;
    PaintType mPaintType = NOPAINT;
    bool mGradientAssigned = false;
    Gradient *mGradient = NULL;
};

class StrokeSvgAttributes : public FillSvgAttributes {
public:
    StrokeSvgAttributes() {}



    StrokeSvgAttributes &operator*=(const StrokeSvgAttributes &overwritter)
    {
        if(overwritter.wasColorAssigned()) {
            mColor = overwritter.getColor();
        }
        if(overwritter.wasPaintTypeAssigned()) {
            mPaintType = overwritter.getPaintType();
        }
        if(overwritter.wasGradientAssigned()) {
            mGradient = overwritter.getGradient();
        }

        if(overwritter.wasLineWidthAssigned()) {
            mLineWidth = overwritter.getLineWidth();
        }
        if(overwritter.wasCapStyleAssigned()) {
            mCapStyle = overwritter.getCapStyle();
        }
        if(overwritter.wasJoinStyleAssigned()) {
            mJoinStyle = overwritter.getJoinStyle();
        }
        if(overwritter.wasOutlineCompositionModeAssigned()) {
            mOutlineCompositionMode = overwritter.getOutlineCompositionMode();
        }
        return *this;
    }

    bool wasLineWidthAssigned() const { return mLineWidthAssigned; }
    bool wasCapStyleAssigned() const { return mCapStyleAssigned; }
    bool wasJoinStyleAssigned() const { return mJoinStyleAssigned; }
    bool wasOutlineCompositionModeAssigned() const { return mOutlineCompositionModeAssigned; }

    const qreal &getLineWidth() const { return mLineWidth; }
    const Qt::PenCapStyle &getCapStyle() const { return mCapStyle; }
    const Qt::PenJoinStyle &getJoinStyle() const { return mJoinStyle; }
    const QPainter::CompositionMode &getOutlineCompositionMode() const { return mOutlineCompositionMode; }

    void setLineWidth(const qreal &val) {
        mLineWidth = val;
        mLineWidthAssigned = true;
    }

    void setCapStyle(const Qt::PenCapStyle &capStyle) {
        mCapStyle = capStyle;
        mCapStyleAssigned = true;
    }

    void setJoinStyle(const Qt::PenJoinStyle &joinStyle) {
        mJoinStyle = joinStyle;
        mJoinStyleAssigned = true;
    }

    void setOutlineCompositionMode(const QPainter::CompositionMode &compMode) {
        mOutlineCompositionMode = compMode;
        mOutlineCompositionModeAssigned = true;
    }

    void apply(BoundingBox *box);
protected:
    bool mLineWidthAssigned = false;
    qreal mLineWidth;
    bool mCapStyleAssigned = false;
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    bool mJoinStyleAssigned = false;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    bool mOutlineCompositionModeAssigned = false;
    QPainter::CompositionMode mOutlineCompositionMode = QPainter::CompositionMode_Source;
};

class BoundingBoxSvgAttributes {
public:
    BoundingBoxSvgAttributes() {}

    BoundingBoxSvgAttributes &operator*=(const BoundingBoxSvgAttributes &overwritter)
    {
        mCombinedTransform *= overwritter.getCombinedTransform();

        mFillAttributes *= overwritter.getFillAttributes();
        mStrokeAttributes *= overwritter.getStrokeAttributes();
        mTextAttributes *= overwritter.getTextAttributes();

        return *this;
    }

    const QMatrix &getCombinedTransform() const { return mCombinedTransform; }
    const FillSvgAttributes &getFillAttributes() const { return mFillAttributes; }
    const StrokeSvgAttributes &getStrokeAttributes() const { return mStrokeAttributes; }
    const TextSvgAttributes &getTextAttributes() const { return mTextAttributes; }

    void loadBoundingBoxAttributes(const QDomElement &element);

    void apply(BoundingBox *box);
protected:
    QString mId;
    QMatrix mCombinedTransform;
    FillSvgAttributes mFillAttributes;
    StrokeSvgAttributes mStrokeAttributes;
    TextSvgAttributes mTextAttributes;
};

//class VectorPathSvgAttributes : public BoundingBoxSvgAttributes {
//public:
//    VectorPathSvgAttributes();

//protected:

//};

class VectorPath;
class Canvas;
class BoxesGroup;

extern void loadBoxesGroup(const QDomElement &groupElement, BoxesGroup *parentGroup, BoundingBoxSvgAttributes *attributes);
extern bool parsePathDataFast(const QString &dataStr, VectorPath *path);
extern void loadVectorPath(const QDomElement &pathElement, BoxesGroup *parentGroup, BoundingBoxSvgAttributes *attributes);
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
