/*
#include "svgimporter.h"
#include "vectorpath.h"

#define QT_INHERIT QLatin1String(qt_inherit_text)

VectorPath *createPathNode(BoxesGroup *parent,
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
}
*/
