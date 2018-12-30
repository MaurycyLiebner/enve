#include "Boxes/textbox.h"
#include <QInputDialog>
#include <QMenu>
#include "GUI/mainwindow.h"
#include "canvas.h"
#include "Animators/gradientpoints.h"
#include "Animators/qstringanimator.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "Animators/effectanimators.h"

TextBox::TextBox() : PathBox(TYPE_TEXT) {
    mPivotAutoAdjust = false;
    setName("text");

    mFillSettings->setCurrentColor(QColor(0, 0, 0));
    mStrokeSettings->setPaintType(PaintType::NOPAINT);

    mText = SPtrCreate(QStringAnimator)("text");
    ca_addChildAnimator(mText);
    ca_prependChildAnimator(mText.data(), mEffectsAnimators);
    mText->prp_setUpdater(SPtrCreate(NodePointUpdater)(this));

    mLinesDist = SPtrCreate(QrealAnimator)(100., 0., 100., 1., "line dist");
    mLinesDist->prp_setUpdater(SPtrCreate(NodePointUpdater)(this));
}

#include <QApplication>
#include <QDesktopWidget>

void TextBox::openTextEditor(QWidget* dialogParent) {
    bool ok;
    QString text =
            QInputDialog::getMultiLineText(
                dialogParent, getName() + " text",
                "Text:", mText->getCurrentTextValue(), &ok);
    if(ok) {
        mText->setCurrentTextValue(text);
    }
}

void TextBox::setFont(const QFont &font) {
    clearAllCache();
    mFont = font;
    scheduleUpdate(Animator::USER_CHANGE);
}

void TextBox::setSelectedFontSize(const qreal &size) {
    QFont newFont = mFont;
    newFont.setPointSizeF(size);
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

QString TextBox::getCurrentTextValue() {
    return mText->getCurrentTextValue();
}

MovablePoint *TextBox::getPointAtAbsPos(const QPointF &absPtPos,
                                        const CanvasMode &currentCanvasMode,
                                        const qreal &canvasScaleInv) {
    return PathBox::getPointAtAbsPos(absPtPos,
                                     currentCanvasMode,
                                     canvasScaleInv);
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

bool TextBox::handleSelectedCanvasAction(QAction *selectedAction,
                                         QWidget* widgetsParent) {
    if(selectedAction->objectName() == "tb_set_text") {
        openTextEditor(widgetsParent);
    } else {
        return false;
    }
    return true;
}

SkPath TextBox::getPathAtRelFrame(const int &relFrame) {
    QPainterPath qPath = QPainterPath();

    qreal linesDistAtFrame = mLinesDist->getCurrentEffectiveValueAtRelFrame(relFrame)*0.01;
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
        yT += fm.height()*linesDistAtFrame;
    }

    // !!! remove so that text stays at the same position after changing string
    // QRectF boundingRect = qPath.boundingRect();
    // qPath.translate(-boundingRect.center());
    //

    return QPainterPathToSkPath(qPath);
}

SkPath TextBox::getPathAtRelFrameF(const qreal &relFrame) {
    QPainterPath qPath = QPainterPath();

    qreal linesDistAtFrame = mLinesDist->getCurrentEffectiveValueAtRelFrameF(relFrame)*0.01;
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
        yT += fm.height()*linesDistAtFrame; // changed distance between lines
    }

    // QRectF boundingRect = qPath.boundingRect();
    // qPath.translate(-boundingRect.center());

    return QPainterPathToSkPath(qPath);
}

void TextBox::setCurrentTextValue(const QString &text) {
    mText->setCurrentTextValue(text);
}

bool TextBox::differenceInEditPathBetweenFrames(
        const int& frame1, const int& frame2) const {
    if(mText->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mLinesDist->prp_differencesBetweenRelFrames(frame1, frame2);
}
