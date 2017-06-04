#include "Boxes/textbox.h"
#include <QInputDialog>
#include "mainwindow.h"
#include "canvas.h"
#include "gradientpoints.h"

TextBox::TextBox(BoxesGroup *parent) :
    PathBox(parent, TYPE_TEXT) {
    setName("text");

    mFillSettings->setCurrentColor(Color(0, 0, 0));
    mStrokeSettings->setPaintType(PaintType::NOPAINT);
}

#include <QSqlError>
int TextBox::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = PathBox::prp_saveToSql(query, parentId);

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


void TextBox::prp_loadFromSql(const int &boundingBoxId) {
    PathBox::prp_loadFromSql(boundingBoxId);

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
        setSelectedFontFamilyAndStyle(query.value(idFontFamily).toString(),
                              query.value(idFontStyle).toString() );
        setSelectedFontSize(query.value(idFontSize).toReal());
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

#include <QApplication>
#include <QDesktopWidget>

void TextBox::openTextEditor() {
    bool ok;
    QString text =
            QInputDialog::getMultiLineText(mMainWindow, getName() + " text",
                                           "Text:", mText, &ok);
    if(ok) {
        setText(text);
        callUpdateSchedulers();
    }
}

void TextBox::setText(QString text) {
    clearAllCache();
    mText = text;
    schedulePathUpdate();
}

void TextBox::setFont(QFont font) {
    clearAllCache();
    mFont = font;
    schedulePathUpdate();
}

void TextBox::setSelectedFontSize(qreal size) {
    clearAllCache();
    mFont.setPointSize(size);
    schedulePathUpdate();
}

void TextBox::setSelectedFontFamilyAndStyle(QString fontFamily,
                                            QString fontStyle) {
    clearAllCache();
    mFont.setFamily(fontFamily);
    mFont.setStyleName(fontStyle);
    schedulePathUpdate();
}

MovablePoint *TextBox::getPointAtAbsPos(const QPointF &absPtPos,
                                  const CanvasMode &currentCanvasMode,
                                  const qreal &canvasScaleInv) {
    return PathBox::getPointAtAbsPos(absPtPos,
                                     currentCanvasMode,
                                     canvasScaleInv);
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
    Q_FOREACH(QString line, lines) {
        qreal lineWidth = fm.width(line);
        if(lineWidth > maxWidth) maxWidth = lineWidth;
    }
    Q_FOREACH(QString line, lines) {
        qreal lineWidth = fm.width(line);
        mPath.addText(textForQPainterPath(mAlignment, lineWidth, maxWidth), yT,
                      mFont, line);
        yT += fm.height();
    }

    QRectF boundingRect = mPath.boundingRect();
    mPath.translate(-boundingRect.center());

    updateOutlinePath();
}
