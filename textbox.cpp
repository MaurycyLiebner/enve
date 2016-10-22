#include "textbox.h"
#include <QInputDialog>
#include "mainwindow.h"

TextBox::TextBox(BoxesGroup *parent) : PathBox(parent, TYPE_TEXT)
{
    setName("text");
}

#include <QSqlError>
int TextBox::saveToSql(int parentId)
{
    int boundingBoxId = PathBox::saveToSql(parentId);

    QSqlQuery query;
    if(!query.exec(QString("INSERT INTO textbox (boundingboxid, "
                           "text, fontfamily, fontstyle, fontsize) "
                "VALUES ('%1', '%2', '%3', '%4', %5)").
                arg(boundingBoxId).
                arg(mText).
                arg(mFont.family()).
                arg(mFont.style()).
                arg(mFont.pointSizeF()) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return boundingBoxId;
}


void TextBox::loadFromSql(int boundingBoxId) {
    PathBox::loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM textbox WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idText = query.record().indexOf("text");
        int idFontFamily = query.record().indexOf("fontfamily");
        int idFontStyle = query.record().indexOf("fontstyle");
        int idFontSize = query.record().indexOf("fontsize");

        setText(query.value(idText).toString());
        setFontFamilyAndStyle(query.value(idFontFamily).toString(),
                              query.value(idFontStyle).toString() );
        setFontSize(query.value(idFontSize).toReal());
    } else {
        qDebug() << "Could not load vectorpath with id " << boundingBoxId;
    }

    if(!mPivotChanged) centerPivotPosition();
}


#include <QApplication>
#include <QDesktopWidget>
QRectF TextBox::getTextRect() {
    QFontMetrics fm(mFont);
    QRectF rect = fm.boundingRect(QApplication::desktop()->geometry(),
                           mAlignment, mText);
    return rect.translated(-rect.topLeft() );
}

bool TextBox::pointInsidePath(QPointF point)
{
    if(mPathText) {
        return PathBox::pointInsidePath(point);
    } else {
        QPainterPath mapped;
        mapped.addRect(getTextRect());
        mapped = mCombinedTransformMatrix.map(mapped);
        return mapped.contains(point);
    }
}

QRectF TextBox::getBoundingRect()
{
    if(mPathText) {
        return PathBox::getBoundingRect();
    } else {
        QRectF rect = mCombinedTransformMatrix.mapRect(getTextRect() );
        return rect;
    }
}

void TextBox::centerPivotPosition() {
    mTransformAnimator.setPivotWithoutChangingTransformation(getTextRect().center() );
}

void TextBox::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();
        if(mPathText) {
            drawBoundingRect(p);
        } else {
            QPainterPath mapped;
            mapped.addRect(getTextRect() );
            mapped = mCombinedTransformMatrix.map(mapped);
            QPen pen = p->pen();
            p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
            p->setBrush(Qt::NoBrush);
            p->drawPath(mapped);
            p->setPen(pen);
        }

        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));

            mFillGradientPoints.drawGradientPoints(p);
            mStrokeGradientPoints.drawGradientPoints(p);
        }

        p->restore();
    }
}


void TextBox::draw(QPainter *p)
{
    if(mPathText) {
        PathBox::draw(p);
    } else {
        if(mVisible) {
            p->save();

            if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
                p->setPen(QPen(QBrush(mDrawStrokeGradient), 1.));
            } else if(mStrokeSettings.getPaintType() == FLATPAINT) {
                p->setPen(mStrokeSettings.getCurrentColor().qcol);
            } else{
                p->setPen(Qt::NoPen);
            }

            p->setOpacity(p->opacity()*mTransformAnimator.getOpacity()*0.01 );
            p->setTransform(QTransform(mCombinedTransformMatrix) );
            p->setFont(mFont);
            p->drawText(getTextRect(), mAlignment, mText);

            p->restore();
        }
    }
}

#include <QApplication>
#include <QDesktopWidget>

void TextBox::openTextEditor()
{
    bool ok;
    QString text =
            QInputDialog::getMultiLineText(mMainWindow, getName() + " text",
                                           "Text:", mText, &ok);
    if(ok) {
        setText(text);

        callUpdateSchedulers();
    }
}

//void TextBox::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
//{
//    if(mVisible) {
//        p->save();

//        QPainterPath mapped;
//        mapped.addRect(getTextRect() );
//        mapped = mCombinedTransformMatrix.map(mapped);
//        QPen pen = p->pen();
//        p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
//        p->setBrush(Qt::NoBrush);
//        p->drawPath(mapped);
//        p->setPen(pen);

//        //drawBoundingRect(p);
//        p->restore();
//    }
//}

void TextBox::setText(QString text)
{
    mText = text;
    schedulePathUpdateIfPathText();
}

void TextBox::setFont(QFont font)
{
    mFont = font;
    schedulePathUpdateIfPathText();
}

void TextBox::setFontSize(qreal size)
{
    mFont.setPointSize(size);
    schedulePathUpdateIfPathText();
}

void TextBox::setFontFamilyAndStyle(QString fontFamily, QString fontStyle)
{
    mFont.setFamily(fontFamily);
    mFont.setStyleName(fontStyle);
    schedulePathUpdateIfPathText();
}

MovablePoint *TextBox::getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode)
{
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints.getPointAt(absPtPos);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints.getPointAt(absPtPos);
        }
    }

    return pointToReturn;
}

qreal textForQPainterPath(Qt::Alignment alignment,
                          qreal lineWidth, qreal maxWidth) {
    if(alignment == Qt::AlignCenter) {
        return (maxWidth - lineWidth)*0.5;
    } else if(alignment == Qt::AlignLeft) {
        return 0.;
    } else {// if(alignment == Qt::AlignRight) {
        return maxWidth - lineWidth;
    }
}

void TextBox::setPathText(bool pathText) {
    mPathText = pathText;
    schedulePathUpdateIfPathText();
}

void TextBox::schedulePathUpdateIfPathText() {
    if(mPathText) {
        schedulePathUpdate();
    }
}

void TextBox::updatePath()
{
    if(mPathText) {
        mPath = QPainterPath();

        QStringList lines = mText.split(QRegExp("\n|\r\n|\r"));
        QFontMetricsF fm(mFont);
        qreal yT = 0.;
        qreal maxWidth = 0.;
        foreach(QString line, lines) {
            qreal lineWidth = fm.width(line);
            if(lineWidth > maxWidth) maxWidth = lineWidth;
        }
        foreach(QString line, lines) {
            qreal lineWidth = fm.width(line);
            mPath.addText(textForQPainterPath(mAlignment, lineWidth, maxWidth), yT,
                          mFont, line);
            yT += fm.height();
        }

        updateMappedPath();
    }
}

//void TextBox::centerPivotPosition() {

//}
