#include "svgimporter.h"
#include "Boxes/vectorpath.h"
#include "Boxes/boxesgroup.h"
#include "canvas.h"
#include "Animators/singlepathanimator.h"

#include "Colors/helpers.h"
//bool getRotationScaleFromMatrixIfNoShear(const QMatrix &matrix,
//                                         qreal *rot,
//                                         qreal *sx, qreal *sy) {
//    qreal m11 = matrix.m11();
//    qreal m12 = matrix.m12();
//    qreal m21 = matrix.m21();
//    qreal m22 = matrix.m22();

//    bool scalesDifferentSign = (isZero(m11) || isZero(m22)) ?
//                                m12*m21 > 0. : m11*m22 < 0.;
//    bool cosSinDifferentSign = (isZero(m11) || isZero(m12)) ?
//                                (m21*m22 > 0.) != scalesDifferentSign :
//                                (m11*m12 < 0.) != scalesDifferentSign;
//    qreal sxAbs = qSqrt(m11*m11 + m21*m21);
//    qreal syAbs = qSqrt(m12*m12 + m22*m22);
//    qreal rotT = 0.;
//    qreal sxT = 1.;
//    qreal syT = 1.;
//    for(int i = 0; i < 2; i++) {
//        sxT = (i == 0) ? -sxAbs : sxAbs;
//        syT = (i == 0) ?
//                     ((scalesDifferentSign) ? syAbs : -syAbs) :
//                     ((scalesDifferentSign) ? -syAbs : syAbs);

//        qreal rotM11T = m11/sxT;
//        qreal rotM12T = m12/syT;
//        qreal rotM21T = m21/sxT;
//        qreal rotM22T = m22/syT;

//        if(!isZero2Dec(rotM11T - rotM22T) ||
//           !isZero2Dec(rotM12T + rotM21T)) {
//            return false;
//        }

//        QPointF rotatedPoint = QMatrix(rotM11T, rotM12T,
//                                       rotM21T, rotM22T,
//                                       0., 0.).map(QPointF(1., 0.));
//        rotT = qAtan2(rotatedPoint.y(), rotatedPoint.x());
//        if(rotT > 2*PI) rotT -= 2*PI;
//        if(cosSinDifferentSign) {
//            if((0.5*PI <= rotT && rotT <= PI) ||
//               (1.5*PI <= rotT && rotT <= 2.*PI)) {
//                break;
//            }
//        } else {
//            if((0. <= rotT && rotT <= 0.5*PI) ||
//               (PI <= rotT && rotT <= 1.5*PI)) {
//                break;
//            }
//        }
//    }

//    *sx = sxT;
//    *sy = syT;
//    *rot = rotT*180./PI;

//    return true;
//}

//bool getTranslationShearRotationScaleFromMatrix(const QMatrix &matrix,
//                                                qreal *tx, qreal *ty,
//                                                qreal *lx, qreal *ly,
//                                                qreal *rot,
//                                                qreal *sx, qreal *sy) {
//    if(getRotationScaleFromMatrixIfNoShear(matrix,
//                                           rot,
//                                           sx, sy) ) {
//        *lx = 0.;
//        *ly = 0.;
//        *tx = matrix.dx();
//        *ty = matrix.dy();

//        return true;
//    } else {
//        qreal m11 = matrix.m11();
//        qreal m12 = matrix.m12();
//        qreal m21 = matrix.m21();
//        qreal m22 = matrix.m22();

//        return false;
//    }
//}

struct SvgAttribute {
    SvgAttribute(const QString &nameValueStr) {
        QStringList nameValueList = nameValueStr.split(":");
        name = nameValueList.first();
        value = nameValueList.last();
    }

    QString getName() const {
        return name;
    }

    QString getValue() const {
        return value;
    }

    QString name;
    QString value;
};

void extractSvgAttributes(const QString &string,
                          QList<SvgAttribute> *attributesList) {
    QStringList attributesStrList = string.split(";");
    Q_FOREACH(const QString &attributeStr, attributesStrList) {
        attributesList->append(SvgAttribute(attributeStr));
    }
}


void loadBoxesGroup(const QDomElement &groupElement, BoxesGroup *parentGroup,
                    BoundingBoxSvgAttributes *attributes) {
    QDomNodeList allRootChildNodes = groupElement.childNodes();
    BoxesGroup *boxesGroup;
    bool hasTransform = attributes->hasTransform();
    if(allRootChildNodes.count() > 1 ||
       hasTransform) {
        boxesGroup = new BoxesGroup(parentGroup);
        attributes->apply(boxesGroup);
    } else {
        boxesGroup = parentGroup;
    }

    for(int i = 0; i < allRootChildNodes.count(); i++) {
        QDomNode iNode = allRootChildNodes.at(i);
        if(iNode.isElement()) {
            loadElement(iNode.toElement(), boxesGroup, attributes);
        }
    }
}

// '0' is 0x30 and '9' is 0x39
static bool isDigit(ushort ch)
{
    static quint16 magic = 0x3ff;
    return ((ch >> 4) == 3) && (magic >> (ch & 15));
}

static qreal toDouble(const QChar *&str)
{
    const int maxLen = 255;//technically doubles can go til 308+ but whatever
    char temp[maxLen+1];
    int pos = 0;

    if (*str == QLatin1Char('-')) {
        temp[pos++] = '-';
        ++str;
    } else if (*str == QLatin1Char('+')) {
        ++str;
    }
    while (isDigit(str->unicode()) && pos < maxLen) {
        temp[pos++] = str->toLatin1();
        ++str;
    }
    if (*str == QLatin1Char('.') && pos < maxLen) {
        temp[pos++] = '.';
        ++str;
    }
    while (isDigit(str->unicode()) && pos < maxLen) {
        temp[pos++] = str->toLatin1();
        ++str;
    }
    bool exponent = false;
    if ((*str == QLatin1Char('e') || *str == QLatin1Char('E')) && pos < maxLen) {
        exponent = true;
        temp[pos++] = 'e';
        ++str;
        if ((*str == QLatin1Char('-') || *str == QLatin1Char('+')) && pos < maxLen) {
            temp[pos++] = str->toLatin1();
            ++str;
        }
        while (isDigit(str->unicode()) && pos < maxLen) {
            temp[pos++] = str->toLatin1();
            ++str;
        }
    }

    temp[pos] = '\0';

    qreal val;
    if (!exponent && pos < 10) {
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
            val = ((qreal)ival)/((qreal)div);
        } else {
            val = ival;
        }
        if (neg)
            val = -val;
    } else {
        val = QByteArray::fromRawData(temp, pos).toDouble();
    }
    return val;
}

static qreal toDouble(const QString &str, bool *ok = NULL) {
    const QChar *c = str.constData();
    qreal res = toDouble(c);
    if (ok) {
        *ok = ((*c) == QLatin1Char('\0'));
    }
    return res;
}

//static qreal toDouble(const QStringRef &str, bool *ok = NULL) {
//    const QChar *c = str.constData();
//    qreal res = toDouble(c);
//    if (ok) {
//        *ok = (c == (str.constData() + str.length()));
//    }
//    return res;
//}

static void parseNumbersArray(const QChar *&str, QVarLengthArray<qreal, 8> &points)
{
    while (str->isSpace())
        ++str;
    while (isDigit(str->unicode()) ||
           *str == QLatin1Char('-') || *str == QLatin1Char('+') ||
           *str == QLatin1Char('.')) {

        points.append(toDouble(str));

        while (str->isSpace())
            ++str;
        if (*str == QLatin1Char(','))
            ++str;

        //eat the rest of space
        while (str->isSpace())
            ++str;
    }
}

bool parsePathDataFast(const QString &dataStr, VectorPathSvgAttributes *attributes)
{
    qreal x0 = 0, y0 = 0;              // starting point
    qreal x = 0, y = 0;                // current point
    char lastMode = 0;
    QPointF ctrlPt;
    const QChar *str = dataStr.constData();
    const QChar *end = str + dataStr.size();

    SvgSeparatePath *lastPath = NULL;
    while (str != end) {
        while (str->isSpace())
            ++str;
        QChar pathElem = *str;
        ++str;
        QChar endc = *end;
        *const_cast<QChar *>(end) = 0; // parseNumbersArray requires 0-termination that QStringRef cannot guarantee
        QVarLengthArray<qreal, 8> arg;
        parseNumbersArray(str, arg);
        *const_cast<QChar *>(end) = endc;
        if (pathElem == QLatin1Char('z') || pathElem == QLatin1Char('Z'))
            arg.append(0);//dummy
        const qreal *num = arg.constData();
        int count = arg.count();
        while (count > 0) {
            qreal offsetX = x;        // correction offsets
            qreal offsetY = y;        // for relative commands
            switch (pathElem.unicode()) {
            case 'm': {
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = x0 = num[0] + offsetX;
                y = y0 = num[1] + offsetY;
                num += 2;
                count -= 2;
                lastPath = attributes->newSeparatePath();
                lastPath->moveTo(QPointF(x0, y0));

                 // As per 1.2  spec 8.3.2 The "moveto" commands
                 // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
                 // the subsequent pairs shall be treated as implicit 'lineto' commands.
                 pathElem = QLatin1Char('l');
            }
                break;
            case 'M': {
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = x0 = num[0];
                y = y0 = num[1];
                num += 2;
                count -= 2;
                lastPath = attributes->newSeparatePath();
                lastPath->moveTo(QPointF(x0, y0));

                // As per 1.2  spec 8.3.2 The "moveto" commands
                // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
                // the subsequent pairs shall be treated as implicit 'lineto' commands.
                pathElem = QLatin1Char('L');
            }
                break;
            case 'z':
            case 'Z': {
                x = x0;
                y = y0;
                count--; // skip dummy
                num++;
                lastPath->closePath();
            }
                break;
            case 'l': {
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = num[0] + offsetX;
                y = num[1] + offsetY;
                num += 2;
                count -= 2;
                lastPath->lineTo(QPointF(x, y));
            }
                break;
            case 'L': {
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = num[0];
                y = num[1];
                num += 2;
                count -= 2;
                lastPath->lineTo(QPointF(x, y));
            }
                break;
            case 'h': {
                x = num[0] + offsetX;
                num++;
                count--;
                lastPath->lineTo(QPointF(x, y));
            }
                break;
            case 'H': {
                x = num[0];
                num++;
                count--;
                lastPath->lineTo(QPointF(x, y));
            }
                break;
            case 'v': {
                y = num[0] + offsetY;
                num++;
                count--;
                lastPath->lineTo(QPointF(x, y));
            }
                break;
            case 'V': {
                y = num[0];
                num++;
                count--;
                lastPath->lineTo(QPointF(x, y));
            }
                break;
            case 'c': {
                if (count < 6) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1(num[0] + offsetX, num[1] + offsetY);
                QPointF c2(num[2] + offsetX, num[3] + offsetY);
                QPointF e(num[4] + offsetX, num[5] + offsetY);
                num += 6;
                count -= 6;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'C': {
                if (count < 6) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1(num[0], num[1]);
                QPointF c2(num[2], num[3]);
                QPointF e(num[4], num[5]);
                num += 6;
                count -= 6;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 's': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1;
                if (lastMode == 'c' || lastMode == 'C' ||
                    lastMode == 's' || lastMode == 'S')
                    c1 = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c1 = QPointF(x, y);
                QPointF c2(num[0] + offsetX, num[1] + offsetY);
                QPointF e(num[2] + offsetX, num[3] + offsetY);
                num += 4;
                count -= 4;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'S': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1;
                if (lastMode == 'c' || lastMode == 'C' ||
                    lastMode == 's' || lastMode == 'S')
                    c1 = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c1 = QPointF(x, y);
                QPointF c2(num[0], num[1]);
                QPointF e(num[2], num[3]);
                num += 4;
                count -= 4;
                lastPath->cubicTo(c1, c2, e);

                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'q': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c(num[0] + offsetX, num[1] + offsetY);
                QPointF e(num[2] + offsetX, num[3] + offsetY);
                num += 4;
                count -= 4;
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'Q': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c(num[0], num[1]);
                QPointF e(num[2], num[3]);
                num += 4;
                count -= 4;
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 't': {
                if (count < 2) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF e(num[0] + offsetX, num[1] + offsetY);
                num += 2;
                count -= 2;
                QPointF c;
                if (lastMode == 'q' || lastMode == 'Q' ||
                    lastMode == 't' || lastMode == 'T')
                    c = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c = QPointF(x, y);
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'T': {
                if (count < 2) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF e(num[0], num[1]);
                num += 2;
                count -= 2;
                QPointF c;
                if (lastMode == 'q' || lastMode == 'Q' ||
                    lastMode == 't' || lastMode == 'T')
                    c = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c = QPointF(x, y);
                lastPath->quadTo(c, e);

                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'a': {
                if (count < 7) {
                    num += count;
                    count = 0;
                    break;
                }
                qreal rx = (*num++);
                qreal ry = (*num++);
                qreal xAxisRotation = (*num++);
                qreal largeArcFlag  = (*num++);
                qreal sweepFlag = (*num++);
                qreal ex = (*num++) + offsetX;
                qreal ey = (*num++) + offsetY;
                count -= 7;
                qreal curx = x;
                qreal cury = y;
                lastPath->pathArc(rx, ry, xAxisRotation, int(largeArcFlag),
                                  int(sweepFlag), ex, ey, curx, cury);

                x = ex;
                y = ey;
            }
                break;
            case 'A': {
                if (count < 7) {
                    num += count;
                    count = 0;
                    break;
                }
                qreal rx = (*num++);
                qreal ry = (*num++);
                qreal xAxisRotation = (*num++);
                qreal largeArcFlag  = (*num++);
                qreal sweepFlag = (*num++);
                qreal ex = (*num++);
                qreal ey = (*num++);
                count -= 7;
                qreal curx = x;
                qreal cury = y;
                lastPath->pathArc(rx, ry, xAxisRotation, int(largeArcFlag),
                                  int(sweepFlag), ex, ey, curx, cury);

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

void BoundingBoxSvgAttributes::loadBoundingBoxAttributes(const QDomElement &element) {
    QList<SvgAttribute> styleAttributes;
    QString styleAttributesStr = element.attribute("style");
    extractSvgAttributes(styleAttributesStr, &styleAttributes);
    Q_FOREACH(const SvgAttribute &attribute, styleAttributes) {
        QString name = attribute.getName();
        if(name.isEmpty()) continue;
        QString value = attribute.getValue();
        if(value == "inherit") continue;
        switch (name.at(0).unicode()) {
        case 'c':
            if (name == "color") {
                mFillAttributes.setPaintType(FLATPAINT);
                Color color = mFillAttributes.getColor();
                color.setGLColorA(toDouble(value));
                mFillAttributes.setColor(color);
            } else if (name == "color-opacity") {
                Color color = mFillAttributes.getColor();
                color.setGLColorA(toDouble(value));
                mFillAttributes.setColor(color);
            } else if (name == "comp-op") {
                //compOp = value;
            }
            break;

        case 'd':
            if (name == "display") {
                //display = value;
            }
            break;

        case 'f':
            if (name == "fill") {
                if(value.contains("none")) {
                    mFillAttributes.setPaintType(NOPAINT);
                } else {
                    mFillAttributes.setPaintType(FLATPAINT);
                    Color color;
                    getColorFromString(value, &color);
                    mFillAttributes.setColor(color);
                }
            } else if (name == "fill-rule") {
                if(value == "nonzero") {
                    mFillRule = Qt::WindingFill;
                } else { // "evenodd"
                    mFillRule = Qt::OddEvenFill;
                }
            } else if (name == "fill-opacity") {
                mFillAttributes.setColorOpacity(toDouble(value));
            } else if (name == "font-family") {
                mTextAttributes.setFontFamily(value);
            } else if (name == "font-size") {
                mTextAttributes.setFontSize(value.toInt());
            } else if (name == "font-style") {
                if(value == "normal") {
                    mTextAttributes.setFontStyle(QFont::StyleNormal);
                } else if(value == "italic") {
                    mTextAttributes.setFontStyle(QFont::StyleItalic);
                } else if(value == "oblique") {
                    mTextAttributes.setFontStyle(QFont::StyleOblique);
                }
            } else if (name == "font-weight") {
                if(value == "normal") {
                    mTextAttributes.setFontWeight(QFont::Normal);
                } else if(value == "bold") {
                    mTextAttributes.setFontWeight(QFont::Bold);
                } else if(value == "bolder") {
                    mTextAttributes.setFontWeight(QFont::ExtraBold);
                } else if(value == "lighter") {
                    mTextAttributes.setFontWeight(QFont::ExtraLight);
                } else if(value == "100") {
                    mTextAttributes.setFontWeight(10);
                } else if(value == "200") {
                    mTextAttributes.setFontWeight(20);
                } else if(value == "300") {
                    mTextAttributes.setFontWeight(30);
                } else if(value == "400") {
                    mTextAttributes.setFontWeight(40);
                } else if(value == "500") {
                    mTextAttributes.setFontWeight(50);
                } else if(value == "600") {
                    mTextAttributes.setFontWeight(60);
                } else if(value == "700") {
                    mTextAttributes.setFontWeight(70);
                } else if(value == "800") {
                    mTextAttributes.setFontWeight(80);
                } else if(value == "900") {
                    mTextAttributes.setFontWeight(90);
                }
            } else if (name == "font-variant") {
                //fontVariant = value;
            }
            break;

        case 'i':
            if (name == "id")
                mId = value;
            break;

        case 'o':
            if (name == "opacity") {
                mOpacity = toDouble(value)*100.;
            } else if (name == "offset") {
                //offset = value;
            }
            break;

        case 's':
            if(name.contains("stroke")) {
                if(name == "stroke") {
                    if(value.contains("none")) {
                        mStrokeAttributes.setPaintType(NOPAINT);
                    } else {
                        mStrokeAttributes.setPaintType(FLATPAINT);
                        Color color;
                        getColorFromString(value, &color);
                        mStrokeAttributes.setColor(color);
                    }
                } else if (name == "stroke-dasharray") {
                    //strokeDashArray = value;
                } else if (name == "stroke-dashoffset") {
                    //strokeDashOffset = value;
                } else if (name == "stroke-linecap") {
                    if(value == "butt") {
                        mStrokeAttributes.setCapStyle(Qt::FlatCap);
                    } else if(value == "round") {
                        mStrokeAttributes.setCapStyle(Qt::RoundCap);
                    } else {
                        mStrokeAttributes.setCapStyle(Qt::SquareCap);
                    }
                } else if (name == "stroke-linejoin") {
                    if(value == "miter") {
                        mStrokeAttributes.setJoinStyle(Qt::MiterJoin);
                    } else if(value == "round") {
                        mStrokeAttributes.setJoinStyle(Qt::RoundJoin);
                    } else {
                        mStrokeAttributes.setJoinStyle(Qt::BevelJoin);
                    }
                } else if (name == "stroke-miterlimit") {
                    //mStrokeAttributes.setMiterLimit(toDouble(value));
                } else if (name == "stroke-opacity") {
                    mStrokeAttributes.setColorOpacity(toDouble(value));
                } else if (name == "stroke-width") {
                    if(value.contains("%")) {
                        mStrokeAttributes.setLineWidth(
                                    mStrokeAttributes.getLineWidth()*
                                    value.toDouble()/100.);
                    } else {
                        value = value.remove("px");
                        mStrokeAttributes.setLineWidth(value.toDouble());
                    }
                }
            } else if (name == "stop-color") {
                //stopColor = value;
            } else if (name == "stop-opacity") {
                //stopOpacity = value;
            }
            break;
        case 't':
            if(name == "text-anchor") {
                if(value == "end") {
                    mTextAttributes.setFontAlignment(Qt::AlignLeft);
                } else if(value == "middle") {
                    mTextAttributes.setFontAlignment(Qt::AlignHCenter);
                } else if(value == "start"){
                    mTextAttributes.setFontAlignment(Qt::AlignRight);
                }
            } else if(name == "transform") {
                mRelTransform = getMatrixFromString(value)*mRelTransform;
            }
            break;

        case 'v':
            if (name == "vector-effect") {
                //vectorEffect = value;
            } else if (name == "visibility") {
                //visibility = value;
            }
            break;

        case 'x':
            if (name == "xml:id") {
                mId = value;
            }
            break;

        default:
            break;
        }
    }

    QString matrixStr = element.attribute("transform");
    if(!matrixStr.isEmpty()) {
        mRelTransform = getMatrixFromString(matrixStr)*mRelTransform;
    }
//    qreal dX, dY, lX, lY, rot, sX, sY;
//    if(getTranslationShearRotationScaleFromMatrix(mRelTransform,
//                                                  &dX, &dY,
//                                                  &lX, &lY,
//                                                  &rot,
//                                                  &sX, &sY)) {
//        mDx = dX;
//        mDy = dY;
//        mRot = rot;
//        mScaleX = sX;
//        mScaleY = sY;

//        QMatrix matrix;
//        matrix.translate(mDx, mDy);
//        matrix.rotate(mRot);
//        matrix.scale(mScaleX, mScaleY);

//        mRelTransform = mRelTransform*matrix.inverted();
//    }
}

void BoundingBoxSvgAttributes::applySingleTransformations(BoundingBox *box) {
    BoxTransformAnimator *animator = box->getTransformAnimator();
    animator->translate(mDx, mDy);
    animator->setScale(mScaleX, mScaleY);
    animator->setRotation(mRot);
}

void loadVectorPath(const QDomElement &pathElement, BoxesGroup *parentGroup,
                    VectorPathSvgAttributes *attributes) {
    VectorPath *vectorPath = new VectorPath(parentGroup);

    QString pathStr = pathElement.attribute("d");
    parsePathDataFast(pathStr, attributes);
    attributes->apply(vectorPath);
}

void loadElement(const QDomElement &element, BoxesGroup *parentGroup,
                 BoundingBoxSvgAttributes *parentGroupAttributes) {
    if(element.tagName() == "g") {
        BoundingBoxSvgAttributes attributes;
        attributes *= (*parentGroupAttributes);
        attributes.loadBoundingBoxAttributes(element);
        loadBoxesGroup(element, parentGroup, &attributes);
    } else if(element.tagName() == "path") {
        VectorPathSvgAttributes attributes;
        attributes *= (*parentGroupAttributes);
        attributes.loadBoundingBoxAttributes(element);
        loadVectorPath(element, parentGroup, &attributes);
    }
}

bool getUrlId(const QString &urlStr, QString *id) {
    QRegExp rx = QRegExp("url\\(\\s*#(.*)\\)", Qt::CaseInsensitive);
    if(rx.exactMatch(urlStr)) {
        rx.indexIn(urlStr);
        QStringList capturedTxt = rx.capturedTexts();
        *id = capturedTxt.at(1);
        return true;
    }

    return false;
}

bool getColorFromString(const QString &colorStr, Color *color) {
    QRegExp rx = QRegExp("rgb\\(.*\\)", Qt::CaseInsensitive);
    if(rx.exactMatch(colorStr)) {
        rx = QRegExp("rgb\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\)", Qt::CaseInsensitive);
        if(rx.exactMatch(colorStr)) {
            rx.indexIn(colorStr);
            QStringList intRGB = rx.capturedTexts();
            color->setRGB(((const QString &)intRGB.at(1)).toInt()/255.,
                          ((const QString &)intRGB.at(2)).toInt()/255.,
                          ((const QString &)intRGB.at(3)).toInt()/255. );
        } else {
            rx = QRegExp("rgb\\(\\s*(\\d+)\\s*%\\s*,\\s*(\\d+)\\s*%\\s*,\\s*(\\d+)\\s*%\\s*\\)", Qt::CaseInsensitive);
            rx.indexIn(colorStr);
            QStringList intRGB = rx.capturedTexts();
            color->setRGB(((const QString &)intRGB.at(1)).toInt()/100.,
                          ((const QString &)intRGB.at(2)).toInt()/100.,
                          ((const QString &)intRGB.at(3)).toInt()/100. );

        }
    } else {
        rx = QRegExp("#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})", Qt::CaseInsensitive);
        if(rx.exactMatch(colorStr)) {
            color->setQColor(QColor(colorStr));
        } else {
            qDebug() << "getColorFromString: Color format not recognised" <<
                        endl << colorStr;
            return false;
        }
    }

    return true;
}

QMatrix getMatrixFromString(const QString &matrixStr) {
    QMatrix matrix;
    if(matrixStr.isEmpty()) return matrix;

    QRegExp rx = QRegExp("matrix\\("
                         "\\s*(-?\\d+(\\.\\d*)?),"
                         "\\s*(-?\\d+(\\.\\d*)?),"
                         "\\s*(-?\\d+(\\.\\d*)?),"
                         "\\s*(-?\\d+(\\.\\d*)?),"
                         "\\s*(-?\\d+(\\.\\d*)?),"
                         "\\s*(-?\\d+(\\.\\d*)?)"
                         "\\)", Qt::CaseInsensitive);
    if(rx.exactMatch(matrixStr)) {
        rx.indexIn(matrixStr);
        QStringList capturedTxt = rx.capturedTexts();
        matrix.setMatrix( ( (const QString&)capturedTxt.at(1)).toDouble(),
                          ( (const QString&)capturedTxt.at(3)).toDouble(),
                          ( (const QString&)capturedTxt.at(5)).toDouble(),
                          ( (const QString&)capturedTxt.at(7)).toDouble(),
                          ( (const QString&)capturedTxt.at(9)).toDouble(),
                          ( (const QString&)capturedTxt.at(11)).toDouble());
    } else {
        QRegExp rx2 = QRegExp("translate\\("
                             "\\s*(-?\\d+(\\.\\d*)?),"
                             "\\s*(-?\\d+(\\.\\d*)?)"
                             "\\)", Qt::CaseInsensitive);
        if(rx2.exactMatch(matrixStr)) {
            rx2.indexIn(matrixStr);
            QStringList capturedTxt = rx2.capturedTexts();
            matrix.translate(((const QString&)capturedTxt.at(1)).toDouble(),
                             ((const QString&)capturedTxt.at(3)).toDouble());
        } else {
            qDebug() << "getMatrixFromString - could not extract values from string: "
                     << endl << matrixStr;
        }
    }

    return matrix;
}
#include "mainwindow.h"
void loadSVGFile(const QString &filename, Canvas *canvas) {
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument document;
        if(document.setContent(&file) ) {
            QDomElement rootElement = document.firstChildElement("svg");
            if(!rootElement.isNull()) {
                BoundingBoxSvgAttributes attributes;
                loadBoxesGroup(rootElement, canvas, &attributes);
            } else {
                qDebug() << "File does not have svg root element";
            }
        } else {
            qDebug() << "Cannot set file as QDomDocument content";
        }
    } else {
        qDebug() << "Cannot open file " + filename;
    }
    MainWindow::getInstance()->callUpdateSchedulers();
}

/*

#define QT_INHERIT QLatin1String(qt_inherit_text)

typedef BoundingBox *(*FactoryMethod)(BoxesGroup *, const QXmlStreamAttributes &, QSvgHandler *);


bool QSvgHandler::startElement(const QString &localName,
                               const QXmlStreamAttributes &attributes)
{
    QSvgNode *node = 0;

    pushColorCopy();

    const QStringRef xmlSpace(attributes.value(QLatin1String("xml:space")));
    if (xmlSpace.isNull()) {
        // This element has no xml:space attribute.
        m_whitespaceMode.push(m_whitespaceMode.isEmpty() ? QSvgText::Default : m_whitespaceMode.top());
    } else if (xmlSpace == QLatin1String("preserve")) {
        m_whitespaceMode.push(QSvgText::Preserve);
    } else if (xmlSpace == QLatin1String("default")) {
        m_whitespaceMode.push(QSvgText::Default);
    } else {
        qWarning() << QString::fromLatin1("\"%1\" is an invalid value for attribute xml:space. "
                                          "Valid values are \"preserve\" and \"default\".").arg(xmlSpace.toString());
        m_whitespaceMode.push(QSvgText::Default);
    }

    if (!m_doc && localName != QLatin1String("svg"))
        return false;

    if (FactoryMethod method = findGroupFactory(localName)) {
        //group
        node = method(m_doc ? m_nodes.top() : 0, attributes, this);
        Q_ASSERT(node);
        if (!m_doc) {
            Q_ASSERT(node->type() == QSvgNode::DOC);
            m_doc = static_cast<QSvgTinyDocument*>(node);
        } else {
            switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS:
            case QSvgNode::SWITCH:
            {
                QSvgStructureNode *group =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                group->addChild(node, someId(attributes));
            }
                break;
            default:
                break;
            }
        }
        parseCoreNode(node, attributes);
#ifndef QT_NO_CSSPARSER
        cssStyleLookup(node, this, m_selector);
#endif
        parseStyle(node, attributes, this);
    } else if (FactoryMethod method = findGraphicsFactory(localName)) {
        //rendering element
        Q_ASSERT(!m_nodes.isEmpty());
        node = method(m_nodes.top(), attributes, this);
        if (node) {
            switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS:
            case QSvgNode::SWITCH:
            {
                QSvgStructureNode *group =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                group->addChild(node, someId(attributes));
            }
                break;
            case QSvgNode::TEXT:
            case QSvgNode::TEXTAREA:
                if (node->type() == QSvgNode::TSPAN) {
                    static_cast<QSvgText *>(m_nodes.top())->addTspan(static_cast<QSvgTspan *>(node));
                } else {
                    qWarning("\'text\' or \'textArea\' element contains invalid element type.");
                    delete node;
                    node = 0;
                }
                break;
            default:
                qWarning("Could not add child element to parent element because the types are incorrect.");
                delete node;
                node = 0;
                break;
            }

            if (node) {
                parseCoreNode(node, attributes);
#ifndef QT_NO_CSSPARSER
                cssStyleLookup(node, this, m_selector);
#endif
                parseStyle(node, attributes, this);
                if (node->type() == QSvgNode::TEXT || node->type() == QSvgNode::TEXTAREA) {
                    static_cast<QSvgText *>(node)->setWhitespaceMode(m_whitespaceMode.top());
                } else if (node->type() == QSvgNode::TSPAN) {
                    static_cast<QSvgTspan *>(node)->setWhitespaceMode(m_whitespaceMode.top());
                }
            }
        }
    } else if (ParseMethod method = findUtilFactory(localName)) {
        Q_ASSERT(!m_nodes.isEmpty());
        if (!method(m_nodes.top(), attributes, this)) {
            qWarning("Problem parsing %s", qPrintable(localName));
        }
    } else if (StyleFactoryMethod method = findStyleFactoryMethod(localName)) {
        QSvgStyleProperty *prop = method(m_nodes.top(), attributes, this);
        if (prop) {
            m_style = prop;
            m_nodes.top()->appendStyleProperty(prop, someId(attributes));
        } else {
            qWarning("Could not parse node: %s", qPrintable(localName));
        }
    } else if (StyleParseMethod method = findStyleUtilFactoryMethod(localName)) {
        if (m_style) {
            if (!method(m_style, attributes, this)) {
                qWarning("Problem parsing %s", qPrintable(localName));
            }
        }
    } else {
        //qWarning()<<"Skipping unknown element!"<<namespaceURI<<"::"<<localName;
        m_skipNodes.push(Unknown);
        return true;
    }

    if (node) {
        m_nodes.push(node);
        m_skipNodes.push(Graphics);
    } else {
        //qDebug()<<"Skipping "<<localName;
        m_skipNodes.push(Style);
    }
    return true;
}


static FactoryMethod findGraphicsFactory(const QString &name)
{
    if (name.isEmpty())
        return 0;

    QStringRef ref(&name, 1, name.length() - 1);
    switch (name.at(0).unicode()) {
    case 'a':
        if (ref == QLatin1String("nimation")) return createAnimationNode;
        break;
    case 'c':
        if (ref == QLatin1String("ircle")) return createCircleNode;
        break;
    case 'e':
        if (ref == QLatin1String("llipse")) return createEllipseNode;
        break;
    case 'i':
        if (ref == QLatin1String("mage")) return createImageNode;
        break;
    case 'l':
        if (ref == QLatin1String("ine")) return createLineNode;
        break;
    case 'p':
        if (ref == QLatin1String("ath")) return createPathNode;
        if (ref == QLatin1String("olygon")) return createPolygonNode;
        if (ref == QLatin1String("olyline")) return createPolylineNode;
        break;
    case 'r':
        if (ref == QLatin1String("ect")) return createRectNode;
        break;
    case 't':
        if (ref == QLatin1String("ext")) return createTextNode;
        if (ref == QLatin1String("extArea")) return createTextAreaNode;
        if (ref == QLatin1String("span")) return createTspanNode;
        break;
    case 'u':
        if (ref == QLatin1String("se")) return createUseNode;
        break;
    case 'v':
        if (ref == QLatin1String("ideo")) return createVideoNode;
        break;
    default:
        break;
    }
    return 0;
}


BoundingBox *createPathNode(BoxesGroup *parent,
                                const QXmlStreamAttributes &attributes)
{
    QStringRef data = attributes.value(QLatin1String("d"));

    VectorPath *newPath = new VectorPath(parent);
    parsePathDataFast(data, newPath);
    return newPath;
}

QVector<qreal> parsePercentageList(const QChar *&str)
{
    QVector<qreal> points;
    if (!str)
        return points;

    while (str->isSpace())
        ++str;
    while ((*str >= QLatin1Char('0') && *str <= QLatin1Char('9')) ||
           *str == QLatin1Char('-') || *str == QLatin1Char('+') ||
           *str == QLatin1Char('.')) {

        points.append(toDouble(str));

        while (str->isSpace())
            ++str;
        if (*str == QLatin1Char('%'))
            ++str;
        while (str->isSpace())
            ++str;
        if (*str == QLatin1Char(','))
            ++str;

        //eat the rest of space
        while (str->isSpace())
            ++str;
    }

    return points;
}

bool resolveColor(const QStringRef &colorStr, QColor &color, QSvgHandler *handler)
{
    QStringRef colorStrTr = trimRef(colorStr);
    if (colorStrTr.isEmpty())
        return false;

    switch(colorStrTr.at(0).unicode()) {

        case '#':
            {
                // #rrggbb is very very common, so let's tackle it here
                // rather than falling back to QColor
                QRgb rgb;
                bool ok = qsvg_get_hex_rgb(colorStrTr.unicode(), colorStrTr.length(), &rgb);
                if (ok)
                    color.setRgb(rgb);
                return ok;
            }
            break;

        case 'r':
            {
                // starts with "rgb(", ends with ")" and consists of at least 7 characters "rgb(,,)"
                if (colorStrTr.length() >= 7 && colorStrTr.at(colorStrTr.length() - 1) == QLatin1Char(')')
                    && QStringRef(colorStrTr.string(), colorStrTr.position(), 4) == QLatin1String("rgb(")) {
                    const QChar *s = colorStrTr.constData() + 4;
                    QVector<qreal> compo = parseNumbersList(s);
                    //1 means that it failed after reaching non-parsable
                    //character which is going to be "%"
                    if (compo.size() == 1) {
                        s = colorStrTr.constData() + 4;
                        compo = parsePercentageList(s);
                        for (int i = 0; i < compo.size(); ++i)
                            compo[i] *= (qreal)2.55;
                    }

                    if (compo.size() == 3) {
                        color = QColor(int(compo[0]),
                                       int(compo[1]),
                                       int(compo[2]));
                        return true;
                    }
                    return false;
                }
            }
            break;

        case 'c':
            if (colorStrTr == QLatin1String("currentColor")) {
                color = handler->currentColor();
                return true;
            }
            break;
        case 'i':
            if (colorStrTr == QT_INHERIT)
                return false;
            break;
        default:
            break;
    }

    color = QColor(colorStrTr.toString());
    return color.isValid();
}

bool constructColor(const QStringRef &colorStr, const QStringRef &opacity,
                           QColor &color, QSvgHandler *handler)
{
    if (!resolveColor(colorStr, color, handler))
        return false;
    if (!opacity.isEmpty()) {
        bool ok = true;
        qreal op = qMin(qreal(1.0), qMax(qreal(0.0), toDouble(opacity, &ok)));
        if (!ok)
            op = 1.0;
        color.setAlphaF(op);
    }
    return true;
}

QColor parseColor(const QSvgAttributes &attributes,
                       QSvgHandler *handler)
{
    QColor color;
    if (constructColor(attributes.color, attributes.colorOpacity, color, handler)) {
        handler->popColor();
        handler->pushColor(color);
    }
    return color;
}

bool parsePathDataFast(const QStringRef &dataStr, VectorPath *path)
{
    qreal x0 = 0, y0 = 0;              // starting point
    qreal x = 0, y = 0;                // current point
    char lastMode = 0;
    QPointF ctrlPt;
    const QChar *str = dataStr.constData();
    const QChar *end = str + dataStr.size();

    PathPoint *firstPointAfterM = NULL;
    PathPoint *lastAddedPoint = NULL;
    while (str != end) {
        while (str->isSpace())
            ++str;
        QChar pathElem = *str;
        ++str;
        QChar endc = *end;
        *const_cast<QChar *>(end) = 0; // parseNumbersArray requires 0-termination that QStringRef cannot guarantee
        QVarLengthArray<qreal, 8> arg;
        parseNumbersArray(str, arg);
        *const_cast<QChar *>(end) = endc;
        if (pathElem == QLatin1Char('z') || pathElem == QLatin1Char('Z'))
            arg.append(0);//dummy
        const qreal *num = arg.constData();
        int count = arg.count();
        while (count > 0) {
            qreal offsetX = x;        // correction offsets
            qreal offsetY = y;        // for relative commands
            if(firstPointAfterM == NULL) {
                firstPointAfterM = lastAddedPoint;
            }
            switch (pathElem.unicode()) {
            case 'm': {
                firstPointAfterM = NULL;
                lastAddedPoint = NULL;
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = x0 = num[0] + offsetX;
                y = y0 = num[1] + offsetY;
                num += 2;
                count -= 2;
                //path.moveTo(x0, y0);

                 // As per 1.2  spec 8.3.2 The "moveto" commands
                 // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
                 // the subsequent pairs shall be treated as implicit 'lineto' commands.
                 pathElem = QLatin1Char('l');
            }
                break;
            case 'M': {
                firstPointAfterM = NULL;
                lastAddedPoint = NULL;
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = x0 = num[0];
                y = y0 = num[1];
                num += 2;
                count -= 2;
                //path.moveTo(x0, y0);

                // As per 1.2  spec 8.3.2 The "moveto" commands
                // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
                // the subsequent pairs shall be treated as implicit 'lineto' commands.
                pathElem = QLatin1Char('L');
            }
                break;
            case 'z':
            case 'Z': {
                x = x0;
                y = y0;
                count--; // skip dummy
                num++;
                if(firstPointAfterM != NULL && lastAddedPoint != NULL) {
                    firstPointAfterM->connectToPoint(lastAddedPoint);
                    firstPointAfterM = NULL;
                    lastAddedPoint = NULL;
                }
            }
                break;
            case 'l': {
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = num[0] + offsetX;
                y = num[1] + offsetY;
                num += 2;
                count -= 2;
                //path.lineTo(x, y);
                // !!!
                lastAddedPoint = path->addPointRelPos(QPointF(x, y), 0, 0, lastAddedPoint);
                // !!!
            }
                break;
            case 'L': {
                if (count < 2) {
                    num++;
                    count--;
                    break;
                }
                x = num[0];
                y = num[1];
                num += 2;
                count -= 2;
                //path.lineTo(x, y);
                // !!!
                lastAddedPoint = path->addPointRelPos(QPointF(x, y), 0, 0, lastAddedPoint);
                // !!!
            }
                break;
            case 'h': {
                x = num[0] + offsetX;
                num++;
                count--;
                //path.lineTo(x, y);
                // !!!
                lastAddedPoint = path->addPointRelPos(QPointF(x, y), 0, 0, lastAddedPoint);
                // !!!
            }
                break;
            case 'H': {
                x = num[0];
                num++;
                count--;
                //path.lineTo(x, y);
                // !!!
                lastAddedPoint = path->addPointRelPos(QPointF(x, y), 0, 0, lastAddedPoint);
                // !!!
            }
                break;
            case 'v': {
                y = num[0] + offsetY;
                num++;
                count--;
                //path.lineTo(x, y);
                // !!!
                lastAddedPoint = path->addPointRelPos(QPointF(x, y), 0, 0, lastAddedPoint);
                // !!!
            }
                break;
            case 'V': {
                y = num[0];
                num++;
                count--;
                //path.lineTo(x, y);
                // !!!
                lastAddedPoint = path->addPointRelPos(QPointF(x, y), 0, 0, lastAddedPoint);
                // !!!
            }
                break;
            case 'c': {
                if (count < 6) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1(num[0] + offsetX, num[1] + offsetY);
                QPointF c2(num[2] + offsetX, num[3] + offsetY);
                QPointF e(num[4] + offsetX, num[5] + offsetY);
                num += 6;
                count -= 6;
                //path.cubicTo(c1, c2, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c2, c1, lastAddedPoint);
                // !!!
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'C': {
                if (count < 6) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1(num[0], num[1]);
                QPointF c2(num[2], num[3]);
                QPointF e(num[4], num[5]);
                num += 6;
                count -= 6;
                //path.cubicTo(c1, c2, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c2, c1, lastAddedPoint);
                // !!!
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 's': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1;
                if (lastMode == 'c' || lastMode == 'C' ||
                    lastMode == 's' || lastMode == 'S')
                    c1 = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c1 = QPointF(x, y);
                QPointF c2(num[0] + offsetX, num[1] + offsetY);
                QPointF e(num[2] + offsetX, num[3] + offsetY);
                num += 4;
                count -= 4;
                //path.cubicTo(c1, c2, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c2, c1, lastAddedPoint);
                // !!!
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'S': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c1;
                if (lastMode == 'c' || lastMode == 'C' ||
                    lastMode == 's' || lastMode == 'S')
                    c1 = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c1 = QPointF(x, y);
                QPointF c2(num[0], num[1]);
                QPointF e(num[2], num[3]);
                num += 4;
                count -= 4;
                //path.cubicTo(c1, c2, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c2, c1, lastAddedPoint);
                // !!!
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                break;
            }
            case 'q': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c(num[0] + offsetX, num[1] + offsetY);
                QPointF e(num[2] + offsetX, num[3] + offsetY);
                num += 4;
                count -= 4;
                //path.quadTo(c, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c,
                                                      QPointF(0.f, 0.f),
                                                      lastAddedPoint);
                // !!!
                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'Q': {
                if (count < 4) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF c(num[0], num[1]);
                QPointF e(num[2], num[3]);
                num += 4;
                count -= 4;
                //path.quadTo(c, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c,
                                                      QPointF(0.f, 0.f),
                                                      lastAddedPoint);
                // !!!
                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 't': {
                if (count < 2) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF e(num[0] + offsetX, num[1] + offsetY);
                num += 2;
                count -= 2;
                QPointF c;
                if (lastMode == 'q' || lastMode == 'Q' ||
                    lastMode == 't' || lastMode == 'T')
                    c = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c = QPointF(x, y);
                //path.quadTo(c, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c,
                                                      QPointF(0.f, 0.f),
                                                      lastAddedPoint);
                // !!!
                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'T': {
                if (count < 2) {
                    num += count;
                    count = 0;
                    break;
                }
                QPointF e(num[0], num[1]);
                num += 2;
                count -= 2;
                QPointF c;
                if (lastMode == 'q' || lastMode == 'Q' ||
                    lastMode == 't' || lastMode == 'T')
                    c = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c = QPointF(x, y);
                //path.quadTo(c, e);
                // !!!
                lastAddedPoint = path->addPointRelPos(e, c,
                                                      QPointF(0.f, 0.f),
                                                      lastAddedPoint);
                // !!!
                ctrlPt = c;
                x = e.x();
                y = e.y();
                break;
            }
            case 'a': {
                if (count < 7) {
                    num += count;
                    count = 0;
                    break;
                }
                qreal rx = (*num++);
                qreal ry = (*num++);
                qreal xAxisRotation = (*num++);
                qreal largeArcFlag  = (*num++);
                qreal sweepFlag = (*num++);
                qreal ex = (*num++) + offsetX;
                qreal ey = (*num++) + offsetY;
                count -= 7;
                qreal curx = x;
                qreal cury = y;
                //pathArc(path, rx, ry, xAxisRotation, int(largeArcFlag),
                //        int(sweepFlag), ex, ey, curx, cury);

                x = ex;
                y = ey;
            }
                break;
            case 'A': {
                if (count < 7) {
                    num += count;
                    count = 0;
                    break;
                }
                qreal rx = (*num++);
                qreal ry = (*num++);
                qreal xAxisRotation = (*num++);
                qreal largeArcFlag  = (*num++);
                qreal sweepFlag = (*num++);
                qreal ex = (*num++);
                qreal ey = (*num++);
                count -= 7;
                qreal curx = x;
                qreal cury = y;
                //pathArc(path, rx, ry, xAxisRotation, int(largeArcFlag),
                //        int(sweepFlag), ex, ey, curx, cury);

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
}*/
#include "Animators/paintsettings.h"
void FillSvgAttributes::apply(BoundingBox *box) {
    PaintSetting *setting;
    if(mPaintType == FLATPAINT) {
        setting = new PaintSetting(true, ColorSetting(RGBMODE,
                                                      CVR_ALL,
                                                      mColor.gl_r,
                                                      mColor.gl_g,
                                                      mColor.gl_b,
                                                      mColor.gl_a,
                                                      CST_CHANGE));
    } else if(mPaintType == GRADIENTPAINT) {
        setting = new PaintSetting(true, mGradient);
    } else {
        setting = new PaintSetting(true);
    }
    if(box->isVectorPath()) {
        setting->apply((PathBox*)box);
    }
    delete setting;
}

void StrokeSvgAttributes::apply(BoundingBox *box, const qreal &scale)
{
    box->setStrokeWidth(mLineWidth*scale, false);

    PaintSetting *setting;
    if(mPaintType == FLATPAINT) {
        setting = new PaintSetting(false, ColorSetting(RGBMODE,
                                                      CVR_ALL,
                                                      mColor.gl_r,
                                                      mColor.gl_g,
                                                      mColor.gl_b,
                                                      mColor.gl_a,
                                                      CST_CHANGE));
    } else if(mPaintType == GRADIENTPAINT) {
        setting = new PaintSetting(false, mGradient);
    } else {
        setting = new PaintSetting(false);
    }
    if(box->isVectorPath()) {
        setting->apply((PathBox*)box);
    }
    delete setting;
    //box->setStrokePaintType(mPaintType, mColor, mGradient);
}

void BoundingBoxSvgAttributes::apply(BoundingBox *box) {
    box->getTransformAnimator()->setOpacity(mOpacity);
}

void VectorPathSvgAttributes::apply(VectorPath *path)
{
    PathAnimator *pathAnimator = path->getPathAnimator();
    Q_FOREACH(SvgSeparatePath *separatePath, mSvgSeparatePaths) {
        separatePath->applyTransfromation(mRelTransform);
        SinglePathAnimator *singlePath =
                new SinglePathAnimator(pathAnimator);
        separatePath->apply(singlePath);
        pathAnimator->addSinglePathAnimator(singlePath);
    }

    qreal m11 = mRelTransform.m11();
    qreal m12 = mRelTransform.m12();
    qreal m21 = mRelTransform.m21();
    qreal m22 = mRelTransform.m22();

    qreal sxAbs = qSqrt(m11*m11 + m21*m21);
    qreal syAbs = qSqrt(m12*m12 + m22*m22);
    mStrokeAttributes.apply(path,
                            (sxAbs + syAbs)*0.5);
    mFillAttributes.apply(path);
    BoundingBoxSvgAttributes::apply((BoundingBox*)path);
}

void SvgSeparatePath::apply(SinglePathAnimator *path)
{
    PathPoint *lastPoint = NULL;
    PathPoint *firstPoint = NULL;
    Q_FOREACH(SvgPathPoint *point, mPoints) {
        PathPoint *newPoint = new PathPoint(path);
        if(firstPoint == NULL) firstPoint = newPoint;
        path->addPoint(newPoint, lastPoint);
        newPoint->setRelativePos(point->getPoint());
        newPoint->setCtrlsMode(point->getCtrlsMode());

        newPoint->moveEndCtrlPtToRelPos(point->getEndPoint());
        newPoint->setEndCtrlPtEnabled(point->getEndPointEnabled());

        newPoint->moveStartCtrlPtToRelPos(point->getStartPoint());
        newPoint->setStartCtrlPtEnabled(point->getStartPointEnabled());

        lastPoint = newPoint;
    }
    if(mClosedPath) {
        lastPoint->connectToPoint(firstPoint);
    }
}
