#include "Boxes/textbox.h"
#include <QInputDialog>
#include "mainwindow.h"
#include "canvas.h"
#include "gradientpoints.h"
#include "Animators/qstringanimator.h"

TextBox::TextBox() :
    PathBox(TYPE_TEXT) {
    setName("text");

    mFillSettings->setCurrentColor(Color(0, 0, 0));
    mStrokeSettings->setPaintType(PaintType::NOPAINT);

    mText = (new QStringAnimator())->ref<QStringAnimator>();
    mText->prp_setName("text");
    ca_addChildAnimator(mText.data());
}

#include <QSqlError>
int TextBox::saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = PathBox::saveToSql(query, parentId);

    int textId = mText->saveToSql(query, parentId);

    if(!query->exec(QString("INSERT INTO textbox (boundingboxid, "
                           "stringanimatorid, fontfamily, fontstyle, fontsize) "
                "VALUES ('%1', '%2', '%3', '%4', %5)").
                arg(boundingBoxId).
                arg(textId).
                arg(mFont.family()).
                arg(mFont.style()).
                arg(mFont.pointSizeF()) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return boundingBoxId;
}

void TextBox::loadFromSql(const int &boundingBoxId) {
    PathBox::loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM textbox WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idText = query.record().indexOf("stringanimatorid");
        int idFontFamily = query.record().indexOf("fontfamily");
        int idFontStyle = query.record().indexOf("fontstyle");
        int idFontSize = query.record().indexOf("fontsize");

        setSelectedFontFamilyAndStyle(query.value(idFontFamily).toString(),
                                      query.value(idFontStyle).toString() );
        setSelectedFontSize(query.value(idFontSize).toReal());
        mText->loadFromSql(query.value(idText).toInt());
    } else {
        qDebug() << "Could not load vectorpath with id " << boundingBoxId;
    }
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

void TextBox::openTextEditor(const bool &saveUndoRedo) {
    bool ok;
    QString text =
            QInputDialog::getMultiLineText(mMainWindow, getName() + " text",
                                           "Text:",
                                           mText->getCurrentTextValue(), &ok);
    if(ok) {
        setText(text, saveUndoRedo);
        callUpdateSchedulers();
    }
}

void TextBox::setText(const QString &text, const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new ChangeTextUndoRedo(this,
                                           mText->getCurrentTextValue(),
                                           text));
    }
    clearAllCache();
    mText->setCurrentTextValue(text);
    scheduleUpdate();
}

void TextBox::setFont(const QFont &font, const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new ChangeFontUndoRedo(this, mFont, font));
    }
    clearAllCache();
    mFont = font;
    scheduleUpdate();
}

void TextBox::setSelectedFontSize(const qreal &size) {
    QFont newFont = mFont;
    newFont.setPointSize(size);
    setFont(newFont);
}

void TextBox::setSelectedFontFamilyAndStyle(const QString &fontFamily,
                                            const QString &fontStyle) {
    QFont newFont = mFont;
    newFont.setFamily(fontFamily);
    newFont.setStyleName(fontStyle);
    setFont(newFont);
}

qreal TextBox::getFontSize() {
    return mFont.pointSize();
}

QString TextBox::getFontFamily() {
    return mFont.family();
}

QString TextBox::getFontStyle() {
    return mFont.styleName();
}

MovablePoint *TextBox::getPointAtAbsPos(const QPointF &absPtPos,
                                        const CanvasMode &currentCanvasMode,
                                        const qreal &canvasScaleInv) {
    return PathBox::getPointAtAbsPos(absPtPos,
                                     currentCanvasMode,
                                     canvasScaleInv);
}

void TextBox::makeDuplicate(Property *targetBox) {
    PathBox::makeDuplicate(targetBox);
    TextBox *textTarget = (TextBox*)targetBox;
    textTarget->setText(mText->getCurrentTextValue());
    textTarget->setFont(mFont);
    textTarget->setTextAlignment(mAlignment);
}

qreal textForQPainterPath(const Qt::Alignment &alignment,
                          const qreal &lineWidth,
                          const qreal &maxWidth) {
    if(alignment == Qt::AlignCenter) {
        return (maxWidth - lineWidth)*0.5;
    } else if(alignment == Qt::AlignLeft) {
        return 0.;
    } else {// if(alignment == Qt::AlignRight) {
        return maxWidth - lineWidth;
    }
}

void TextBox::addActionsToMenu(QMenu *menu) {
    menu->addAction("Set Text...")->setObjectName("tb_set_text");
}

bool TextBox::handleSelectedCanvasAction(QAction *selectedAction) {
    if(selectedAction->objectName() == "tb_set_text") {
        openTextEditor();
    } else {
        return false;
    }
    return true;
}

SkPath TextBox::getPathAtRelFrame(const int &relFrame) {
    QPainterPath qPath = QPainterPath();

    QString textAtFrame = mText->getTextValueAtRelFrame(relFrame);
    QStringList lines = textAtFrame.split(QRegExp("\n|\r\n|\r"));
    QFontMetricsF fm(mFont);
    qreal yT = 0.;
    qreal maxWidth = 0.;
    Q_FOREACH(QString line, lines) {
        qreal lineWidth = fm.width(line);
        if(lineWidth > maxWidth) maxWidth = lineWidth;
    }
    Q_FOREACH(QString line, lines) {
        qreal lineWidth = fm.width(line);
        qPath.addText(textForQPainterPath(mAlignment, lineWidth, maxWidth),
                      yT, mFont, line);
        yT += fm.height();
    }

    QRectF boundingRect = qPath.boundingRect();
    qPath.translate(-boundingRect.center());

    bool firstOther;
    SkPoint endPt;
    SkPoint startPt;
    SkPath path;
    for(int i = 0; i < qPath.elementCount(); i++) {
        const QPainterPath::Element &elem = qPath.elementAt(i);

        if(elem.isMoveTo()) { // move
            path.moveTo(elem.x, elem.y);
        } else if(elem.isLineTo()) { // line
            path.lineTo(elem.x, elem.y);
        } else if(elem.isCurveTo()) { // curve
            endPt = SkPoint::Make(elem.x, elem.y);
            firstOther = true;
        } else { // other
            if(firstOther) {
                startPt = SkPoint::Make(elem.x, elem.y);
            } else {
                path.cubicTo(endPt, startPt, SkPoint::Make(elem.x, elem.y));
            }
            firstOther = !firstOther;
        }
    }
    return path;
}
