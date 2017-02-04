#include "Boxes/textbox.h"
#include <QInputDialog>
#include "mainwindow.h"

TextBox::TextBox(BoxesGroup *parent) : PathBox(parent, TYPE_TEXT)
{
    setName("text");
}

#include <QSqlError>
int TextBox::saveToSql(QSqlQuery *query, int parentId)
{
    int boundingBoxId = PathBox::saveToSql(query, parentId);

    if(!query->exec(QString("INSERT INTO textbox (boundingboxid, "
                           "text, fontfamily, fontstyle, fontsize) "
                "VALUES ('%1', '%2', '%3', '%4', %5)").
                arg(boundingBoxId).
                arg(mText).
                arg(mFont.family()).
                arg(mFont.style()).
                arg(mFont.pointSizeF()) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
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


//#include <QApplication>
//#include <QDesktopWidget>
//QRectF TextBox::getTextRect() {
//    QFontMetrics fm(mFont);
//    QRectF rect = fm.boundingRect(QApplication::desktop()->geometry(),
//                           mAlignment, mText);
//    return rect.translated(QPointF(0., -fm.height()));
//}

void TextBox::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));

            mFillGradientPoints.drawGradientPoints(p);
            mStrokeGradientPoints.drawGradientPoints(p);
        }

        p->restore();
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
    }
}

void TextBox::setText(QString text)
{
    mText = text;
    schedulePathUpdate();
}

void TextBox::setFont(QFont font)
{
    mFont = font;
    schedulePathUpdate();
}

void TextBox::setFontSize(qreal size)
{
    mFont.setPointSize(size);
    schedulePathUpdate();
}

void TextBox::setFontFamilyAndStyle(QString fontFamily, QString fontStyle)
{
    mFont.setFamily(fontFamily);
    mFont.setStyleName(fontStyle);
    schedulePathUpdate();
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

void TextBox::updatePath()
{
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

    QRectF boundingRect = mPath.boundingRect();
    mPath.translate(-boundingRect.center());

    updateOutlinePath();
}
