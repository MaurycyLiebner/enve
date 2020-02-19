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

#include "svgimporter.h"
#include "Boxes/containerbox.h"
#include "colorhelpers.h"
#include "pointhelpers.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "Boxes/textbox.h"
#include "Animators/transformanimator.h"
#include "paintsettingsapplier.h"
#include "Boxes/smartvectorpath.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "MovablePoints/smartnodepoint.h"
#include "Private/document.h"
#include "matrixdecomposition.h"
#include "transformvalues.h"

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
    PaintType mPaintType = NOPAINT;
    Gradient *mGradient = nullptr;
    QPointF mGradientP1;
    QPointF mGradientP2;
    QMatrix mGradientTransform;
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

    SkPathFillType getFillRule() const;
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
    SkPathFillType mFillRule = SkPathFillType::kEvenOdd;

    TransformValues mDecomposedTrans;

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

    void removeInvalidPaths() {
        for(int i = mSeparatePaths.count() - 1; i >= 0; i--) {
            const auto& path = mSeparatePaths.at(i);
            const bool valid = path.countVerbs() > 1;
            if(!valid) mSeparatePaths.removeAt(i);
        }
    }

    bool isEmpty() const {
        return mSeparatePaths.isEmpty();
    }
protected:
    QList<SkPath> mSeparatePaths;
};

struct SvgAttribute {
    SvgAttribute(const QString &nameValueStr) {
        const QStringList nameValueList = nameValueStr.split(":");
        fName = nameValueList.first();
        fValue = nameValueList.last();
    }

    QString getName() const {
        return fName;
    }

    QString getValue() const {
        return fValue;
    }

    QString fName;
    QString fValue;
};

void extractSvgAttributes(const QString &string,
                          QList<SvgAttribute> * const attributesList) {
    const QStringList attributesStrList = string.split(";");
    for(const QString &attributeStr : attributesStrList) {
        attributesList->append(SvgAttribute(attributeStr));
    }
}


bool toColor(const QString &colorStr, QColor &color) {
    QRegExp rx = QRegExp("rgb\\(.*\\)", Qt::CaseInsensitive);
    if(rx.exactMatch(colorStr)) {
        rx = QRegExp("rgb\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\)", Qt::CaseInsensitive);
        if(rx.exactMatch(colorStr)) {
            rx.indexIn(colorStr);
            QStringList intRGB = rx.capturedTexts();
            color.setRgb(intRGB.at(1).toInt(),
                         intRGB.at(2).toInt(),
                         intRGB.at(3).toInt());
        } else {
            rx = QRegExp("rgb\\(\\s*(\\d+)\\s*%\\s*,\\s*(\\d+)\\s*%\\s*,\\s*(\\d+)\\s*%\\s*\\)", Qt::CaseInsensitive);
            rx.indexIn(colorStr);
            QStringList intRGB = rx.capturedTexts();
            color.setRgbF(intRGB.at(1).toInt()/100.,
                          intRGB.at(2).toInt()/100.,
                          intRGB.at(3).toInt()/100.);

        }
    } else {
        rx = QRegExp("#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})", Qt::CaseInsensitive);
        if(rx.exactMatch(colorStr)) {
            color = QColor(colorStr);
        } else {
            return false;
        }
    }
    return true;
}

// '0' is 0x30 and '9' is 0x39
static bool isDigit(ushort ch) {
    static quint16 magic = 0x3ff;
    return ((ch >> 4) == 3) && (magic >> (ch & 15));
}

static qreal toDouble(const QChar *&str) {
    const int maxLen = 255;//technically doubles can go til 308+ but whatever
    char temp[maxLen+1];
    int pos = 0;

    if(*str == QLatin1Char('-')) {
        temp[pos++] = '-';
        ++str;
    } else if(*str == QLatin1Char('+')) {
        ++str;
    }
    while(isDigit(str->unicode()) && pos < maxLen) {
        temp[pos++] = str->toLatin1();
        ++str;
    }
    if(*str == QLatin1Char('.') && pos < maxLen) {
        temp[pos++] = '.';
        ++str;
    }
    while(isDigit(str->unicode()) && pos < maxLen) {
        temp[pos++] = str->toLatin1();
        ++str;
    }
    bool exponent = false;
    if((*str == QLatin1Char('e') || *str == QLatin1Char('E')) && pos < maxLen) {
        exponent = true;
        temp[pos++] = 'e';
        ++str;
        if((*str == QLatin1Char('-') || *str == QLatin1Char('+')) && pos < maxLen) {
            temp[pos++] = str->toLatin1();
            ++str;
        }
        while(isDigit(str->unicode()) && pos < maxLen) {
            temp[pos++] = str->toLatin1();
            ++str;
        }
    }

    temp[pos] = '\0';

    qreal val;
    if(!exponent && pos < 10) {
        int ival = 0;
        const char *t = temp;
        bool neg = false;
        if(*t == '-') {
            neg = true;
            ++t;
        }
        while(*t && *t != '.') {
            ival *= 10;
            ival += (*t) - '0';
            ++t;
        }
        if(*t == '.') {
            ++t;
            int div = 1;
            while(*t) {
                ival *= 10;
                ival += (*t) - '0';
                div *= 10;
                ++t;
            }
            val = static_cast<qreal>(ival)/div;
        } else {
            val = ival;
        }
        if(neg) val = -val;
    } else {
        val = QByteArray::fromRawData(temp, pos).toDouble();
    }
    return val;
}

static qreal toDouble(const QString &str, bool *ok = nullptr) {
    const QChar *c = str.constData();
    const qreal res = toDouble(c);
    if(ok) *ok = (*c == QLatin1Char('\0'));
    return res;
}

//static qreal toDouble(const QStringRef &str, bool *ok = nullptr) {
//    const QChar *c = str.constData();
//    qreal res = toDouble(c);
//    if(ok) {
//        *ok = (c == (str.constData() + str.length()));
//    }
//    return res;
//}

static void parseNumbersArray(const QChar *&str,
                              QVarLengthArray<float, 8> &points) {
    while(str->isSpace()) str++;
    while(isDigit(str->unicode()) ||
          *str == QLatin1Char('-') || *str == QLatin1Char('+') ||
          *str == QLatin1Char('.')) {

        points.append(toSkScalar(toDouble(str)));

        while(str->isSpace()) str++;
        if(*str == QLatin1Char(',')) str++;

        //eat the rest of space
        while(str->isSpace()) str++;
    }
}

bool parsePathDataFast(const QString &dataStr,
                       VectorPathSvgAttributes &attributes) {
    float x0 = 0, y0 = 0;              // starting point
    float x = 0, y = 0;                // current point
    char lastMode = 0;
    SkPoint ctrlPt{0, 0};
    const QChar *str = dataStr.constData();
    const QChar *end = str + dataStr.size();

    SkPath *lastPath = nullptr;
    while(str != end) {
        while(str->isSpace()) str++;
        QChar pathElem = *str;
        ++str;
        QChar endc = *end;
        *const_cast<QChar *>(end) = 0; // parseNumbersArray requires 0-termination that QStringRef cannot guarantee
        QVarLengthArray<float, 8> arg;
        parseNumbersArray(str, arg);
        *const_cast<QChar *>(end) = endc;
        if(pathElem == QLatin1Char('z') || pathElem == QLatin1Char('Z'))
            arg.append(0);//dummy
        const float *num = arg.constData();
        int count = arg.count();
        while (count > 0) {
            float offsetX = x;        // correction offsets
            float offsetY = y;        // for relative commands
            switch (pathElem.unicode()) {
            case 'm': {
                if(count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = x0 = num[0] + offsetX;
                y = y0 = num[1] + offsetY;
                num += 2;
                count -= 2;
                lastPath = &attributes.newPath();
                lastPath->moveTo({x0, y0});

                pathElem = QLatin1Char('l');
            }
                break;
            case 'M': {
                if(count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = x0 = num[0];
                y = y0 = num[1];
                num += 2;
                count -= 2;
                lastPath = &attributes.newPath();
                lastPath->moveTo({x0, y0});

                pathElem = QLatin1Char('L');
            }
                break;
            case 'z':
            case 'Z': {
                x = x0;
                y = y0;
                count--; // skip dummy
                num++;
                lastPath->close();
            }
                break;
            case 'l': {
                if(count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = num[0] + offsetX;
                y = num[1] + offsetY;
                num += 2;
                count -= 2;
                lastPath->lineTo({x, y});
            }
                break;
            case 'L': {
                if(count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = num[0];
                y = num[1];
                num += 2;
                count -= 2;
                lastPath->lineTo({x, y});
            }
                break;
            case 'h': {
                x = num[0] + offsetX;
                num++;
                count--;
                lastPath->lineTo({x, y});
            }
                break;
            case 'H': {
                x = num[0];
                num++;
                count--;
                lastPath->lineTo({x, y});
            }
                break;
            case 'v': {
                y = num[0] + offsetY;
                num++;
                count--;
                lastPath->lineTo({x, y});
            }
                break;
            case 'V': {
                y = num[0];
                num++;
                count--;
                lastPath->lineTo({x, y});
            }
                break;
            case 'c': {
                if(count < 6) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint c1{num[0] + offsetX, num[1] + offsetY};
                SkPoint c2{num[2] + offsetX, num[3] + offsetY};
                SkPoint e{num[4] + offsetX, num[5] + offsetY};
                num += 6;
                count -= 6;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'C': {
                if(count < 6) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint c1{num[0], num[1]};
                SkPoint c2{num[2], num[3]};
                SkPoint e{num[4], num[5]};
                num += 6;
                count -= 6;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 's': {
                if(count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint c1;
                if(lastMode == 'c' || lastMode == 'C' ||
                    lastMode == 's' || lastMode == 'S')
                    c1 = {2*x-ctrlPt.x(), 2*y-ctrlPt.y()};
                else c1 = {x, y};
                SkPoint c2{num[0] + offsetX, num[1] + offsetY};
                SkPoint e{num[2] + offsetX, num[3] + offsetY};
                num += 4;
                count -= 4;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'S': {
                if(count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint c1;
                if(lastMode == 'c' || lastMode == 'C' ||
                   lastMode == 's' || lastMode == 'S')
                    c1 = {2*x - ctrlPt.x(), 2*y - ctrlPt.y()};
                else c1 = {x, y};
                SkPoint c2{num[0], num[1]};
                SkPoint e{num[2], num[3]};
                num += 4;
                count -= 4;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'q': {
                if(count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint c{num[0] + offsetX, num[1] + offsetY};
                SkPoint e{num[2] + offsetX, num[3] + offsetY};
                num += 4;
                count -= 4;
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'Q': {
                if(count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint c{num[0], num[1]};
                SkPoint e{num[2], num[3]};
                num += 4;
                count -= 4;
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 't': {
                if(count < 2) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint e{num[0] + offsetX, num[1] + offsetY};
                num += 2;
                count -= 2;
                SkPoint c;
                if(lastMode == 'q' || lastMode == 'Q' ||
                   lastMode == 't' || lastMode == 'T')
                    c = {2*x - ctrlPt.x(), 2*y - ctrlPt.y()};
                else c = {x, y};
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'T': {
                if(count < 2) {
                    num += count;
                    count = 0;
                    break;
                }
                SkPoint e{num[0], num[1]};
                num += 2;
                count -= 2;
                SkPoint c;
                if(lastMode == 'q' || lastMode == 'Q' ||
                    lastMode == 't' || lastMode == 'T')
                    c = {2*x - ctrlPt.x(), 2*y - ctrlPt.y()};
                else
                    c = {x, y};
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'a': {
                if(count < 7) {
                    num += count;
                    count = 0;
                    break;
                }
                const float rx = (*num++);
                const float ry = (*num++);
                const float xAxisRotation = (*num++);
                const auto largeArcFlag  = SkPath::ArcSize(*num++);
                const auto sweepFlag = SkPathDirection(static_cast<int>(*num++) ? 0 : 1);
                const float ex = (*num++) + offsetX;
                const float ey = (*num++) + offsetY;
                count -= 7;
                lastPath->arcTo(rx, ry, xAxisRotation, largeArcFlag,
                                sweepFlag, ex, ey);

                x = ex;
                y = ey;
            }
                break;
            case 'A': {
                if(count < 7) {
                    num += count;
                    count = 0;
                    break;
                }
                const float rx = (*num++);
                const float ry = (*num++);
                const float xAxisRotation = (*num++);
                const auto largeArcFlag  = SkPath::ArcSize(*num++);
                const auto sweepFlag = SkPathDirection(static_cast<int>(*num++) ? 0 : 1);
                const float ex = (*num++);
                const float ey = (*num++);
                count -= 7;
                lastPath->arcTo(rx, ry, xAxisRotation, largeArcFlag,
                                sweepFlag, ex, ey);

                x = ex;
                y = ey;
            }
                break;
            default:
                return false;
            }
            lastMode = pathElem.toLatin1();
        }
    }
    return true;
}


bool parsePolylineDataFast(const QString &dataStr,
                           VectorPathSvgAttributes &attributes) {
    float x0 = 0, y0 = 0;              // starting point
    float x = 0, y = 0;                // current point
    const QChar *str = dataStr.constData();
    const QChar *end = str + dataStr.size();

    SkPath *lastPath = nullptr;
    while (str != end) {
        while (str->isSpace())
            ++str;
        QChar endc = *end;
        *const_cast<QChar *>(end) = 0; // parseNumbersArray requires 0-termination that QStringRef cannot guarantee
        QVarLengthArray<float, 8> arg;
        parseNumbersArray(str, arg);
        *const_cast<QChar *>(end) = endc;
        const float *num = arg.constData();
        int count = arg.count();
        bool first = true;
        while (count > 0) {
            x = num[0];
            y = num[1];
            num++;
            num++;
            count -= 2;
            if(count < 0) {
                if(qAbs(x - x0) < 0.001f &&
                   qAbs(y - y0) < 0.001f) {
                    lastPath->close();
                    return true;
                }
            }
            if(first) {
                x0 = x;
                y0 = y;
                lastPath = &attributes.newPath();
                lastPath->moveTo({x0, y0});
                first = false;
            } else {
                lastPath->lineTo({x, y});
            }
        }
    }
    return true;
}

void loadElement(const QDomElement &element, ContainerBox *parentGroup,
                 const BoxSvgAttributes &parentGroupAttributes,
                 const GradientCreator& gradientCreator);

qsptr<ContainerBox> loadBoxesGroup(const QDomElement &groupElement,
                                   ContainerBox *parentGroup,
                                   const BoxSvgAttributes &attributes,
                                   const GradientCreator& gradientCreator) {
    const QDomNodeList allRootChildNodes = groupElement.childNodes();
    qsptr<ContainerBox> boxesGroup;
    const bool hasTransform = attributes.hasTransform();
    if(allRootChildNodes.count() > 1 || hasTransform || !parentGroup) {
        boxesGroup = enve::make_shared<ContainerBox>(eBoxType::group);
        boxesGroup->planCenterPivotPosition();
        attributes.apply(boxesGroup.get());
        if(parentGroup) parentGroup->addContained(boxesGroup);
    } else {
        boxesGroup = parentGroup->ref<ContainerBox>();
    }

    for(int i = 0; i < allRootChildNodes.count(); i++) {
        const QDomNode iNode = allRootChildNodes.at(i);
        if(iNode.isElement()) {
            loadElement(iNode.toElement(), boxesGroup.get(),
                        attributes, gradientCreator);
        }
    }
    return boxesGroup;
}

void loadVectorPath(const QDomElement &pathElement,
                    ContainerBox *parentGroup,
                    VectorPathSvgAttributes& attributes) {
    const QString pathStr = pathElement.attribute("d");
    parsePathDataFast(pathStr, attributes);
    attributes.removeInvalidPaths();
    if(attributes.isEmpty()) return;
    const auto vectorPath = enve::make_shared<SmartVectorPath>();
    vectorPath->planCenterPivotPosition();
    attributes.apply(vectorPath.get());
    parentGroup->addContained(vectorPath);
}

void loadPolyline(const QDomElement &pathElement,
                  ContainerBox *parentGroup,
                  VectorPathSvgAttributes &attributes) {
    const QString pathStr = pathElement.attribute("points");
    parsePolylineDataFast(pathStr, attributes);
    attributes.removeInvalidPaths();
    if(attributes.isEmpty()) return;
    const auto vectorPath = enve::make_shared<SmartVectorPath>();
    vectorPath->planCenterPivotPosition();
    attributes.apply(vectorPath.get());
    parentGroup->addContained(vectorPath);
}

void loadCircle(const QDomElement &pathElement,
                ContainerBox *parentGroup,
                const BoxSvgAttributes &attributes) {

    const QString cXstr = pathElement.attribute("cx");
    const QString cYstr = pathElement.attribute("cy");
    const QString rStr = pathElement.attribute("r");
    const QString rXstr = pathElement.attribute("rx");
    const QString rYstr = pathElement.attribute("ry");

    qsptr<Circle> circle;
    double rX, rY;
    if(!rStr.isEmpty()) {
        rX = rStr.toDouble();
        rY = rX;
    } else if(!rXstr.isEmpty() && !rYstr.isEmpty()) {
        rX = rXstr.toDouble();
        rY = rYstr.toDouble();
    } else if(!rXstr.isEmpty() || !rYstr.isEmpty()) {
        const qreal rXY = rXstr.isEmpty() ? rYstr.toDouble() : rXstr.toDouble();
        rX = rXY;
        rY = rXY;
    } else return;
    if(isZero4Dec(rX) || isZero4Dec(rY)) return;
    circle = enve::make_shared<Circle>();
    circle->setHorizontalRadius(rX);
    circle->setVerticalRadius(rY);
    circle->setCenter(QPointF(cXstr.toDouble(), cYstr.toDouble()));
    circle->planCenterPivotPosition();

    attributes.apply(circle.data());
    parentGroup->addContained(circle);
}

void loadRect(const QDomElement &pathElement,
              ContainerBox *parentGroup,
              const BoxSvgAttributes &attributes) {

    const QString xStr = pathElement.attribute("x");
    const QString yStr = pathElement.attribute("y");
    const QString wStr = pathElement.attribute("width");
    const QString hStr = pathElement.attribute("height");
    const QString rYstr = pathElement.attribute("ry");
    const QString rXstr = pathElement.attribute("rx");

    const auto rect = enve::make_shared<Rectangle>();
    rect->planCenterPivotPosition();

    const auto topLeft = QPointF(xStr.toDouble(), yStr.toDouble());
    rect->setTopLeftPos(topLeft);
    rect->setBottomRightPos(topLeft + QPointF(wStr.toDouble(), hStr.toDouble()));
    if(rYstr.isEmpty()) {
        rect->setYRadius(rXstr.toDouble());
        rect->setXRadius(rXstr.toDouble());
    } else if(rXstr.isEmpty()) {
        rect->setYRadius(rYstr.toDouble());
        rect->setXRadius(rYstr.toDouble());
    } else {
        rect->setYRadius(rYstr.toDouble());
        rect->setXRadius(rXstr.toDouble());
    }

    attributes.apply(rect.data());
    parentGroup->addContained(rect);
}


void loadText(const QDomElement &pathElement,
              ContainerBox *parentGroup,
              const BoxSvgAttributes &attributes) {

    const QString xStr = pathElement.attribute("x");
    const QString yStr = pathElement.attribute("y");

    const auto textBox = enve::make_shared<TextBox>();
    textBox->planCenterPivotPosition();

    textBox->moveByRel(QPointF(xStr.toDouble(), yStr.toDouble()));
    textBox->setCurrentValue(pathElement.text());

    attributes.apply(textBox.data());
    parentGroup->addContained(textBox);
}

#define eNUMERIC_SVG "\\s*(-?[\\.|\\d]+)"


bool extractTranslation(const QString& str, QMatrix& target) {
    const QRegExp rx1("translate\\("
                      eNUMERIC_SVG
                      "\\)", Qt::CaseInsensitive);
    if(rx1.exactMatch(str)) {
        rx1.indexIn(str);
        const QStringList capturedTxt = rx1.capturedTexts();
        target.translate(capturedTxt.at(1).toDouble(), 0);
        return true;
    }

    const QRegExp rx2("translate\\("
                      eNUMERIC_SVG"[,|\\s+]?"
                      eNUMERIC_SVG
                      "\\)", Qt::CaseInsensitive);
    if(rx2.exactMatch(str)) {
        rx2.indexIn(str);
        const QStringList capturedTxt = rx2.capturedTexts();
        target.translate(capturedTxt.at(1).toDouble(),
                         capturedTxt.at(2).toDouble());
        return true;
    }

    return false;
}


bool extractScale(const QString& str, QMatrix& target) {
    const QRegExp rx1("scale\\("
                      eNUMERIC_SVG
                      "\\)", Qt::CaseInsensitive);
    if(rx1.exactMatch(str)) {
        rx1.indexIn(str);
        const QStringList capturedTxt = rx1.capturedTexts();
        const qreal scale = capturedTxt.at(1).toDouble();
        target.scale(scale, scale);
        return true;
    }

    const QRegExp rx2("scale\\("
                      eNUMERIC_SVG"[,|\\s+]?"
                      eNUMERIC_SVG
                      "\\)", Qt::CaseInsensitive);
    if(rx2.exactMatch(str)) {
        rx2.indexIn(str);
        const QStringList capturedTxt = rx2.capturedTexts();
        target.scale(capturedTxt.at(1).toDouble(),
                     capturedTxt.at(2).toDouble());
        return true;
    }

    return false;
}

bool extractRotate(const QString& str, QMatrix& target) {
    const QRegExp rx5("rotate\\("
                      eNUMERIC_SVG
                      "\\)", Qt::CaseInsensitive);
    if(rx5.exactMatch(str)) {
        rx5.indexIn(str);
        const QStringList capturedTxt = rx5.capturedTexts();
        target.rotate(capturedTxt.at(1).toDouble());
        return true;
    }
    return false;
}

bool extractWholeMatrix(const QString& str, QMatrix& target) {
    const QRegExp rx("matrix\\("
                     eNUMERIC_SVG"[,|\\s+]?"
                     eNUMERIC_SVG"[,|\\s+]?"
                     eNUMERIC_SVG"[,|\\s+]?"
                     eNUMERIC_SVG"[,|\\s+]?"
                     eNUMERIC_SVG"[,|\\s+]?"
                     eNUMERIC_SVG
                     "\\)", Qt::CaseInsensitive);
    if(rx.exactMatch(str)) {
        rx.indexIn(str);
        const QStringList capturedTxt = rx.capturedTexts();
        target.setMatrix(capturedTxt.at(1).toDouble(),
                         capturedTxt.at(2).toDouble(),
                         capturedTxt.at(3).toDouble(),
                         capturedTxt.at(4).toDouble(),
                         capturedTxt.at(5).toDouble(),
                         capturedTxt.at(6).toDouble());
        return true;
    }
    return false;
}

QMatrix getMatrixFromString(const QString &str) {
    QMatrix matrix;
    if(str.isEmpty()) return matrix;
    const bool found = str.isEmpty() ||
                       extractWholeMatrix(str, matrix) ||
                       extractTranslation(str, matrix) ||
                       extractScale(str, matrix) ||
                       extractRotate(str, matrix);
    if(!found) qDebug() << "getMatrixFromString - could not extract "
                           "values from string:" << endl << str;
    return matrix;
}

static QMap<QString, SvgGradient> gGradients;
//            to       from
static QMap<QString, QStringList> gUnresolvedGradientLinks;
void loadElement(const QDomElement &element, ContainerBox *parentGroup,
                 const BoxSvgAttributes &parentGroupAttributes,
                 const GradientCreator& gradientCreator) {
    const QString tagName = element.tagName();
    if(tagName == "defs") {
        const QDomNodeList allRootChildNodes = element.childNodes();
        for(int i = 0; i < allRootChildNodes.count(); i++) {
            const QDomNode iNode = allRootChildNodes.at(i);
            if(iNode.isElement()) {
                loadElement(iNode.toElement(), parentGroup,
                            parentGroupAttributes, gradientCreator);
            }
        }
        return;
    } else if(tagName == "linearGradient") {
        const QString id = element.attribute("id");
        QString linkId = element.attribute("xlink:href");
        Gradient* gradient = nullptr;
        if(linkId.isEmpty()) {
            gradient = gradientCreator();
            const QDomNodeList allRootChildNodes = element.childNodes();
            for(int i = 0; i < allRootChildNodes.count(); i++) {
                const QDomNode iNode = allRootChildNodes.at(i);
                if(!iNode.isElement()) continue;
                const QDomElement elem = iNode.toElement();
                if(elem.tagName() != "stop") continue;
                QColor stopColor;
                const QString stopStyle = elem.attribute("style");
                QList<SvgAttribute> attributesList;
                extractSvgAttributes(stopStyle, &attributesList);
                for(const auto& attr : attributesList) {
                    if(attr.fName == "stop-color") {
                        const qreal alpha = stopColor.alphaF();
                        toColor(attr.fValue, stopColor);
                        stopColor.setAlphaF(alpha);

                    } else if(attr.fName == "stop-opacity") {
                        stopColor.setAlphaF(toDouble(attr.fValue));
                    }
                }
                gradient->addColor(stopColor);
            }
        } else {
            if(linkId.at(0) == "#") linkId.remove(0, 1);
            const auto it = gGradients.find(linkId);
            if(it == gGradients.end()) {
                gUnresolvedGradientLinks[linkId].append(id);
                gradient = nullptr;
            } else {
                gradient = it.value().fGradient;
            }
        }
        const auto it = gUnresolvedGradientLinks.find(id);
        if(it != gUnresolvedGradientLinks.end()) {
            if(gradient) {
                for(const auto& linking : it.value()) {
                    gGradients[linking].fGradient = gradient;
                }
            } else {
                gUnresolvedGradientLinks[linkId] = it.value();
            }
        }

        const QString x1 = element.attribute("x1");
        const QString y1 = element.attribute("y1");
        const QString x2 = element.attribute("x2");
        const QString y2 = element.attribute("y2");
        const QString gradTrans = element.attribute("gradientTransform");
        const QMatrix trans = getMatrixFromString(gradTrans);
        gGradients.insert(id, {gradient,
                               toDouble(x1), toDouble(y1),
                               toDouble(x2), toDouble(y2),
                               trans});
    }
    if(tagName == "path" || tagName == "polyline") {
        VectorPathSvgAttributes attributes;
        attributes.setParent(parentGroupAttributes);
        attributes.loadBoundingBoxAttributes(element);
        if(tagName == "path") {
            loadVectorPath(element, parentGroup, attributes);
        } else { // if(tagName == "polyline") {
            loadPolyline(element, parentGroup, attributes);
        }
    } else if(tagName == "g" || tagName == "text" ||
              tagName == "circle" || tagName == "ellipse" ||
              tagName == "rect" || tagName == "tspan") {
        BoxSvgAttributes attributes;
        attributes.setParent(parentGroupAttributes);
        attributes.loadBoundingBoxAttributes(element);
        if(tagName == "g" || tagName == "text") {
            const auto group = loadBoxesGroup(element, parentGroup,
                                              attributes, gradientCreator);
            if(group->getContainedBoxesCount() == 0)
                group->removeFromParent_k();
        } else if(tagName == "circle" || tagName == "ellipse") {
            loadCircle(element, parentGroup, attributes);
        } else if(tagName == "rect") {
            loadRect(element, parentGroup, attributes);
        } else if(tagName == "tspan") {
            loadText(element, parentGroup, attributes);
        }
    }
}

bool getUrlId(const QString &urlStr, QString *id) {
    const QRegExp rx = QRegExp("url\\(\\s*#(.*)\\)", Qt::CaseInsensitive);
    if(rx.exactMatch(urlStr)) {
        rx.indexIn(urlStr);
        const QStringList capturedTxt = rx.capturedTexts();
        *id = capturedTxt.at(1);
        return true;
    }

    return false;
}

bool getGradientFromString(const QString &colorStr,
                           FillSvgAttributes * const target) {
    const QRegExp rx = QRegExp("url\\(\\s*(.*)\\s*\\)", Qt::CaseInsensitive);
    if(rx.exactMatch(colorStr)) {
        const QStringList capturedTxt = rx.capturedTexts();
        QString id = capturedTxt.at(1);
        if(id.at(0) == '#') id.remove(0, 1);
        const auto it = gGradients.find(id);
        if(it != gGradients.end()) {
            target->setGradient(it.value());
            return true;
        }
    }
    return false;
}

bool getFlatColorFromString(const QString &colorStr, FillSvgAttributes *target) {
    QColor color;
    if(!toColor(colorStr, color)) return false;
    target->setColor(color);
    target->setPaintType(FLATPAINT);
    return true;
}

qsptr<BoundingBox> loadSVGFile(const QString &filename,
                               const GradientCreator& gradientCreator) {
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument document;
        if(document.setContent(&file) ) {
            const QDomElement rootElement = document.firstChildElement("svg");
            if(!rootElement.isNull()) {
                BoxSvgAttributes attributes;
                const auto result = loadBoxesGroup(rootElement, nullptr,
                                                   attributes, gradientCreator);
                gGradients.clear();
                auto it = gUnresolvedGradientLinks.begin();
                while(it != gUnresolvedGradientLinks.end()) {
                    qDebug() << "unresolved gradient links to " + it.key() + ":";
                    qDebug() << it.value().join(", ");
                    it++;
                }
                gUnresolvedGradientLinks.clear();
                if(result->getContainedBoxesCount() == 1) {
                    return qSharedPointerCast<BoundingBox>(
                                result->takeContained_k(0));
                } else if(result->getContainedBoxesCount() == 0) {
                    return nullptr;
                }
                return result;
            } else {
                RuntimeThrow("File does not have svg root element");
            }
        } else {
            RuntimeThrow("Cannot set file as QDomDocument content");
        }
    } else {
        RuntimeThrow("Cannot open file " + filename);
    }
}

void BoxSvgAttributes::setParent(const BoxSvgAttributes &parent) {
    mFillAttributes = parent.getFillAttributes();
    mStrokeAttributes = parent.getStrokeAttributes();
    mTextAttributes = parent.getTextAttributes();
    mFillRule = parent.getFillRule();
}

SkPathFillType BoxSvgAttributes::getFillRule() const {
    return mFillRule;
}

const QMatrix &BoxSvgAttributes::getRelTransform() const {
    return mRelTransform;
}

const FillSvgAttributes &BoxSvgAttributes::getFillAttributes() const {
    return mFillAttributes;
}

const StrokeSvgAttributes &BoxSvgAttributes::getStrokeAttributes() const {
    return mStrokeAttributes;
}

const TextSvgAttributes &BoxSvgAttributes::getTextAttributes() const {
    return mTextAttributes;
}

void BoxSvgAttributes::setFillAttribute(const QString &value) {
    if(value.contains("none")) {
        mFillAttributes.setPaintType(NOPAINT);
    } else if(getFlatColorFromString(value, &mFillAttributes)) {
    } else if(getGradientFromString(value, &mFillAttributes)) {
    } else {
        qDebug() << "setFillAttribute - format not recognised:" <<
                    endl << value;
    }
}

void BoxSvgAttributes::setStrokeAttribute(const QString &value) {
    if(value.contains("none")) {
        mStrokeAttributes.setPaintType(NOPAINT);
    } else if(getFlatColorFromString(value, &mStrokeAttributes)) {
    } else if(getGradientFromString(value, &mStrokeAttributes)) {
    } else {
        qDebug() << "setStrokeAttribute - format not recognised:" <<
                    endl << value;
    }
}

QString stripPx(const QString& val) {
    QString result = val;
    result.remove("px");
    return result;
}

void BoxSvgAttributes::loadBoundingBoxAttributes(const QDomElement &element) {
    QList<SvgAttribute> styleAttributes;
    const QString styleAttributesStr = element.attribute("style");
    extractSvgAttributes(styleAttributesStr, &styleAttributes);
    for(const SvgAttribute &attribute : styleAttributes) {
        const QString name = attribute.getName();
        if(name.isEmpty()) continue;
        const QString value = attribute.getValue();
        if(value == "inherit") continue;
        switch (name.at(0).unicode()) {
        case 'c':
            if(name == "color") {
                mFillAttributes.setPaintType(FLATPAINT);
                QColor color = mFillAttributes.getColor();
                color.setAlphaF(toDouble(value));
                mFillAttributes.setColor(color);
            } else if(name == "color-opacity") {
                QColor color = mFillAttributes.getColor();
                color.setAlphaF(toDouble(value));
                mFillAttributes.setColor(color);
            } else if(name == "comp-op") {
                //compOp = value;
            }
            break;

        case 'd':
            if(name == "display") {
                //display = value;
            }
            break;

        case 'f':
            if(name == "fill") {
                setFillAttribute(value);
            } else if(name == "fill-rule") {
                if(value == "nonzero") {
                    mFillRule = SkPathFillType::kWinding;
                } else { // "evenodd"
                    mFillRule = SkPathFillType::kEvenOdd;
                }
            } else if(name == "fill-opacity") {
                mFillAttributes.setColorOpacity(toDouble(value));
            } else if(name == "font-family") {
                QString stripQuotes = value;
                stripQuotes.remove("'");
                mTextAttributes.setFontFamily(stripQuotes);
            } else if(name == "font-size") {
                mTextAttributes.setFontSize(qRound(stripPx(value).toDouble()));
            } else if(name == "font-style") {
                if(value == "normal") {
                    mTextAttributes.setFontStyle(QFont::StyleNormal);
                } else if(value == "italic") {
                    mTextAttributes.setFontStyle(QFont::StyleItalic);
                } else if(value == "oblique") {
                    mTextAttributes.setFontStyle(QFont::StyleOblique);
                }
            } else if(name == "font-weight") {
                if(value == "normal") {
                    mTextAttributes.setFontWeight(QFont::Normal);
                } else if(value == "bold") {
                    mTextAttributes.setFontWeight(QFont::Bold);
                } else if(value == "bolder") {
                    mTextAttributes.setFontWeight(QFont::ExtraBold);
                } else if(value == "lighter") {
                    mTextAttributes.setFontWeight(QFont::ExtraLight);
                } else {
                    bool ok;
                    const int val = value.toInt(&ok);
                    if(ok) mTextAttributes.setFontWeight(val/10);
                    else qDebug() << "Unrecognized font-weight '" + value + "'";
                }
            } else if(name == "font-variant") {
                //fontVariant = value;
            }
            break;

        case 'i':
            if(name == "id") mId = value;
            break;

        case 'o':
            if(name == "opacity") {
                mOpacity = toDouble(value)*100.;
            } else if(name == "offset") {
                //offset = value;
            }
            break;

        case 's':
            if(name.contains("stroke")) {
                if(name == "stroke") {
                    setStrokeAttribute(value);
                } else if(name == "stroke-dasharray") {
                    //strokeDashArray = value;
                } else if(name == "stroke-dashoffset") {
                    //strokeDashOffset = value;
                } else if(name == "stroke-linecap") {
                    if(value == "butt") {
                        mStrokeAttributes.setCapStyle(SkPaint::kButt_Cap);
                    } else if(value == "round") {
                        mStrokeAttributes.setCapStyle(SkPaint::kRound_Cap);
                    } else {
                        mStrokeAttributes.setCapStyle(SkPaint::kSquare_Cap);
                    }
                } else if(name == "stroke-linejoin") {
                    if(value == "miter") {
                        mStrokeAttributes.setJoinStyle(SkPaint::kMiter_Join);
                    } else if(value == "round") {
                        mStrokeAttributes.setJoinStyle(SkPaint::kRound_Join);
                    } else {
                        mStrokeAttributes.setJoinStyle(SkPaint::kBevel_Join);
                    }
                } else if(name == "stroke-miterlimit") {
                    //mStrokeAttributes.setMiterLimit(toDouble(value));
                } else if(name == "stroke-opacity") {
                    mStrokeAttributes.setColorOpacity(toDouble(value));
                } else if(name == "stroke-width") {
                    if(value.contains("%")) {
                        mStrokeAttributes.setLineWidth(
                                    mStrokeAttributes.getLineWidth()*
                                    value.toDouble()/100.);
                    } else {
                        mStrokeAttributes.setLineWidth(
                                    stripPx(value).toDouble());
                    }
                }
            } else if(name == "stop-color") {
                //stopColor = value;
            } else if(name == "stop-opacity") {
                //stopOpacity = value;
            }
            break;
        case 't':
            if(name == "text-anchor") {
                if(value == "end") {
                    mTextAttributes.setFontAlignment(Qt::AlignLeft);
                } else if(value == "middle") {
                    mTextAttributes.setFontAlignment(Qt::AlignHCenter);
                } else if(value == "start") {
                    mTextAttributes.setFontAlignment(Qt::AlignRight);
                }
            } else if(name == "transform") {
                mRelTransform = getMatrixFromString(value)*mRelTransform;
            }
            break;

        case 'v':
            if(name == "vector-effect") {
                //vectorEffect = value;
            } else if(name == "visibility") {
                //visibility = value;
            }
            break;

        case 'x':
            if(name == "xml:id") {
                mId = value;
            }
            break;

        default:
            break;
        }
    }

    const QString fillAttributesStr = element.attribute("fill");
    if(!fillAttributesStr.isEmpty()) setFillAttribute(fillAttributesStr);

    const QString fillOp = element.attribute("fill-opacity");
    if(!fillOp.isEmpty()) mFillAttributes.setColorOpacity(toDouble(fillOp));

    const QString strokeAttributesStr = element.attribute("stroke");
    if(!strokeAttributesStr.isEmpty()) setStrokeAttribute(strokeAttributesStr);

    const QString strokeOp = element.attribute("stroke-opacity");
    if(!strokeOp.isEmpty()) mFillAttributes.setColorOpacity(toDouble(strokeOp));

    const QString matrixStr = element.attribute("transform");
//    const QString transCenterX = element.attribute("inkscape:transform-center-x");
//    const QString transCenterY = element.attribute("inkscape:transform-center-y");
//    QPointF transCenter;
//    if(!transCenterX.isEmpty()) transCenter.setX(toDouble(transCenterX));
//    if(!transCenterY.isEmpty()) transCenter.setY(toDouble(transCenterY));
    if(!matrixStr.isEmpty()) {
        mRelTransform = getMatrixFromString(matrixStr)*mRelTransform;
    }

    mDecomposedTrans = MatrixDecomposition::decompose(mRelTransform);
}

bool BoxSvgAttributes::hasTransform() const {
    return !(isZero4Dec(mRelTransform.dx()) &&
             isZero4Dec(mRelTransform.dy()) &&
             isZero4Dec(mRelTransform.m11() - 1) &&
             isZero4Dec(mRelTransform.m22() - 1) &&
             isZero4Dec(mRelTransform.m12()) &&
             isZero4Dec(mRelTransform.m21()));
}

#include "Animators/paintsettingsanimator.h"

void FillSvgAttributes::setColor(const QColor &val) {
    mColor = val;
    mColor.setAlphaF(val.alphaF()*mOpacity);
    setPaintType(FLATPAINT);
}

void FillSvgAttributes::setColorOpacity(const qreal opacity) {
    mColor.setAlphaF(opacity);
    mOpacity = opacity;
}

void FillSvgAttributes::setPaintType(const PaintType type) {
    mPaintType = type;
}

void FillSvgAttributes::setGradient(const SvgGradient& gradient) {
    mGradient = gradient.fGradient;
    mGradientP1 = gradient.fTrans.map(QPointF{gradient.fX1, gradient.fY1});
    mGradientP2 = gradient.fTrans.map(QPointF{gradient.fX2, gradient.fY2});
    mGradientTransform = gradient.fTrans;
    if(!mGradient) return;
    setPaintType(GRADIENTPAINT);
}

const QColor &FillSvgAttributes::getColor() const { return mColor; }

PaintType FillSvgAttributes::getPaintType() const { return mPaintType; }

Gradient *FillSvgAttributes::getGradient() const { return mGradient; }

void FillSvgAttributes::apply(BoundingBox *box) const {
    apply(box, PaintSetting::FILL);
}

void FillSvgAttributes::apply(BoundingBox * const box,
                              const PaintSetting::Target& target) const {
    const auto pathBox = enve_cast<PathBox*>(box);
    if(!pathBox) return;
    if(mPaintType == FLATPAINT) {
        ColorSetting colorSetting(ColorMode::rgb, ColorParameter::all,
                                  mColor.redF(), mColor.greenF(),
                                  mColor.blueF(), mColor.alphaF(),
                                  ColorSettingType::change);
        ColorPaintSetting(target, colorSetting).apply(pathBox);
    } else if(mPaintType == GRADIENTPAINT) {
        GradientPaintSetting(target, mGradient).apply(pathBox);
        GradientPtsPosSetting(target, mGradientP1, mGradientP2).apply(pathBox);
        GradientTransformSetting(target, mGradientTransform).apply(pathBox);
    }
    PaintTypePaintSetting(target, mPaintType).apply(pathBox);
}

qreal StrokeSvgAttributes::getLineWidth() const {
    return mLineWidth;
}

SkPaint::Cap StrokeSvgAttributes::getCapStyle() const {
    return mCapStyle;
}

SkPaint::Join StrokeSvgAttributes::getJoinStyle() const {
    return mJoinStyle;
}

QPainter::CompositionMode StrokeSvgAttributes::getOutlineCompositionMode() const {
    return mOutlineCompositionMode;
}

void StrokeSvgAttributes::setLineWidth(const qreal val) {
    mLineWidth = val;
}

void StrokeSvgAttributes::setCapStyle(const SkPaint::Cap capStyle) {
    mCapStyle = capStyle;
}

void StrokeSvgAttributes::setJoinStyle(const SkPaint::Join joinStyle) {
    mJoinStyle = joinStyle;
}

void StrokeSvgAttributes::setOutlineCompositionMode(const QPainter::CompositionMode compMode) {
    mOutlineCompositionMode = compMode;
}

void StrokeSvgAttributes::apply(BoundingBox *box, const qreal scale) const {
    box->strokeWidthAction(QrealAction::sMakeSet(mLineWidth*scale));
    FillSvgAttributes::apply(box, PaintSetting::OUTLINE);
    //box->setStrokePaintType(mPaintType, mColor, mGradient);
}

void BoxSvgAttributes::apply(BoundingBox *box) const {
    if(const auto path = enve_cast<PathBox*>(box)) {
        const qreal m11 = mRelTransform.m11();
        const qreal m12 = mRelTransform.m12();
        const qreal m21 = mRelTransform.m21();
        const qreal m22 = mRelTransform.m22();

        const qreal sxAbs = qSqrt(m11*m11 + m21*m21);
        const qreal syAbs = qSqrt(m12*m12 + m22*m22);
        mStrokeAttributes.apply(path, (sxAbs + syAbs)*0.5);
        mFillAttributes.apply(path);
        if(const auto text = enve_cast<TextBox*>(box)) {
            text->setFont(mTextAttributes.getFont());
        }
    }
    const auto transAnim = box->getBoxTransformAnimator();
    transAnim->setOpacity(mOpacity);
    transAnim->translate(mDecomposedTrans.fMoveX, mDecomposedTrans.fMoveY);
    transAnim->setScale(mDecomposedTrans.fScaleX, mDecomposedTrans.fScaleY);
    transAnim->setRotation(mDecomposedTrans.fRotation);
    transAnim->setShear(mDecomposedTrans.fShearX, mDecomposedTrans.fShearY);
}

void VectorPathSvgAttributes::apply(SmartVectorPath * const path) {
    SmartPathCollection* const pathAnimator = path->getPathAnimator();
//    for(const auto& separatePath : mSvgSeparatePaths) {
//        const auto singlePath = enve::make_shared<SmartPathAnimator>();
//        separatePath->apply(singlePath.get());
//        pathAnimator->addChild(singlePath);
//    }
    for(const auto& separatePath : mSeparatePaths) {
        const auto singlePath = enve::make_shared<SmartPathAnimator>(separatePath);
        pathAnimator->addChild(singlePath);
    }
    pathAnimator->setFillType(mFillRule);
    BoxSvgAttributes::apply(path);
}

void TextSvgAttributes::setFontFamily(const QString &family) {
    mFont.setFamily(family);
}

void TextSvgAttributes::setFontSize(const int size) {
    mFont.setPointSize(size > 0 ? size : 1);
}

void TextSvgAttributes::setFontStyle(const QFont::Style &style) {
    mFont.setStyle(style);
}

void TextSvgAttributes::setFontWeight(const int weight) {
    mFont.setWeight(weight);
}

void TextSvgAttributes::setFontAlignment(const Qt::Alignment &alignment) {
    mAlignment = alignment;
}
