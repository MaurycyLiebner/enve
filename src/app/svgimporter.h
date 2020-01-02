// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H
#include <QDebug>
#include <QtXml/QDomDocument>
#include <QtMath>
#include "paintsettingsapplier.h"
class VectorPath;
class Canvas;
class ContainerBox;
class BoundingBox;
enum class CtrlsMode : short;

class TextSvgAttributes {
public:
    TextSvgAttributes() {}

    void setFontFamily(const QString &family);
    void setFontSize(const int size);
    void setFontStyle(const QFont::Style &style);
    void setFontWeight(const int weight);

    void setFontAlignment(const Qt::Alignment &alignment);

    const QFont &getFont() const { return mFont; }
private:
    Qt::Alignment mAlignment = Qt::AlignLeft;
    QFont mFont;
};

struct SvgGradient {
    Gradient* fGradient;
    qreal fX1;
    qreal fY1;
    qreal fX2;
    qreal fY2;
    QMatrix fTrans;
};

class FillSvgAttributes {
public:
    FillSvgAttributes() {}

    void setColor(const QColor &val);

    void setColorOpacity(const qreal opacity);

    void setPaintType(const PaintType type);

    void setGradient(const SvgGradient& gradient);

    const QColor &getColor() const;
    PaintType getPaintType() const;
    Gradient *getGradient() const;

    void apply(BoundingBox * const box) const;
    void apply(BoundingBox * const box,
               const PaintSetting::Target& target) const;
protected:
    qreal mOpacity = 1;
    QColor mColor;
    PaintType mPaintType = FLATPAINT;//NOPAINT;
    Gradient *mGradient = nullptr;
    QPointF mGradientP1;
    QPointF mGradientP2;
};

class StrokeSvgAttributes : public FillSvgAttributes {
public:
    StrokeSvgAttributes() {}

    qreal getLineWidth() const;
    SkPaint::Cap getCapStyle() const;
    SkPaint::Join getJoinStyle() const;
    QPainter::CompositionMode getOutlineCompositionMode() const;

    void setLineWidth(const qreal val);

    void setCapStyle(const SkPaint::Cap capStyle);
    void setJoinStyle(const SkPaint::Join joinStyle);

    void setOutlineCompositionMode(const QPainter::CompositionMode compMode);

    void apply(BoundingBox *box, const qreal scale) const;
protected:
    SkPaint::Cap mCapStyle = SkPaint::kRound_Cap;
    SkPaint::Join mJoinStyle = SkPaint::kRound_Join;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
    qreal mLineWidth = 0;
};

class BoxSvgAttributes {
public:
    void setParent(const BoxSvgAttributes &parent);

    SkPath::FillType getFillRule() const;
    const QMatrix &getRelTransform() const;
    const FillSvgAttributes &getFillAttributes() const;
    const StrokeSvgAttributes &getStrokeAttributes() const;
    const TextSvgAttributes &getTextAttributes() const;

    void loadBoundingBoxAttributes(const QDomElement &element);

    bool hasTransform() const;

    void apply(BoundingBox *box) const;
    void setFillAttribute(const QString &value);
    void setStrokeAttribute(const QString &value);
protected:
    void decomposeTransformMatrix();

    SkPath::FillType mFillRule = SkPath::kEvenOdd_FillType;

    qreal mDx = 0;
    qreal mDy = 0;
    qreal mScaleX = 1;
    qreal mScaleY = 1;
    qreal mShearX = 0;
    qreal mShearY = 0;
    qreal mRot = 0;

    qreal mOpacity = 100;

    QMatrix mRelTransform;

    QString mId;

    FillSvgAttributes mFillAttributes;
    StrokeSvgAttributes mStrokeAttributes;
    TextSvgAttributes mTextAttributes;
};

class PathAnimator;
class VectorPathAnimator;
class SmartPathAnimator;

class SmartVectorPath;
class VectorPathSvgAttributes : public BoxSvgAttributes {
public:
    SkPath& newPath() {
        mSeparatePaths << SkPath();
        return mSeparatePaths.last();
    }

    void apply(SmartVectorPath * const path);
protected:
    QList<SkPath> mSeparatePaths;
};


extern void loadElement(const QDomElement &element, ContainerBox *parentGroup,
                        const BoxSvgAttributes &parentGroupAttributes);
extern qsptr<BoundingBox> loadSVGFile(const QString &filename);

#endif // SVGIMPORTER_H
