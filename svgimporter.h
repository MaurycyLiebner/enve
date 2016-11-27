#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H
#include <QDebug>
#include <QtXml/QDomDocument>
#include "fillstrokesettings.h"

class TextSvgAttributes {
public:
    TextSvgAttributes();

private:
};

class FillSvgAttributes {
public:
    FillSvgAttributes();

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

    bool wasColorAssigned() const { return mColorAssigned; }
    bool wasPaintTypeAssigned() const { return mPaintTypeAssigned; }
    bool wasGradientAssigned() const { return mGradientAssigned; }

    Color getColor() const { return mColor; }
    PaintType getPaintType() const { return mPaintType; }
    Gradient *getGradient() const { return mGradient; }
protected:
    bool mColorAssigned = false;
    Color mColor;
    bool mPaintTypeAssigned = false;
    PaintType mPaintType = FLATPAINT;
    bool mGradientAssigned = false;
    Gradient *mGradient = NULL;
};

class StrokeSvgAttributes : public FillSvgAttributes {
public:
    StrokeSvgAttributes();

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

    qreal getLineWidth() const { return mLineWidth; }
    Qt::PenCapStyle getCapStyle() const { return mCapStyle; }
    Qt::PenJoinStyle getJoinStyle() const { return mJoinStyle; }
    QPainter::CompositionMode getOutlineCompositionMode() const { return mOutlineCompositionMode; }
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
    BoundingBoxSvgAttributes();

protected:
    QString mId;
    QMatrix mCombinedTransform;
    FillSvgAttributes mFillAttributes;
    StrokeSvgAttributes mStrokeAttributes;
    TextSvgAttributes mTextAttributes;
};

class VectorPathSvgAttributes : public BoundingBoxSvgAttributes {
public:
    VectorPathSvgAttributes();

protected:

};

class VectorPath;
class Canvas;
class BoxesGroup;

extern void loadBoxesGroup(const QDomElement &groupElement, BoxesGroup *parentGroup);
extern bool parsePathDataFast(const QString &dataStr, VectorPath *path);
extern void loadVectorPath(const QDomElement &pathElement, BoxesGroup *parentGroup);
extern void loadElement(const QDomElement &element, BoxesGroup *parentGroup);
extern void loadSVGFile(const QString &filename, Canvas *canvas);

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
