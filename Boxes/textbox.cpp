#include "Boxes/textbox.h"
#include <QInputDialog>
#include "mainwindow.h"
#include "canvas.h"
#include "gradientpoints.h"
#include "Animators/qstringanimator.h"
#include "Animators/animatorupdater.h"

TextBox::TextBox() : PathBox(TYPE_TEXT) {
    mPivotAutoAdjust = false;
    setName("text");

    mFillSettings->setCurrentColor(Color(0, 0, 0));
    mStrokeSettings->setPaintType(PaintType::NOPAINT);

    mText = (new QStringAnimator())->ref<QStringAnimator>();
    mText->prp_setName("text");
    ca_addChildAnimator(mText.data());
    ca_prependChildAnimator(mText.data(), mEffectsAnimators.data());
    mText->prp_setUpdater(new NodePointUpdater(this));

    mLinesDist = (new QrealAnimator())->ref<QrealAnimator>();
    mLinesDist->prp_setName("line dist");
    mLinesDist->qra_setValueRange(0., 100.);
    mLinesDist->qra_setCurrentValue(100.);
    mLinesDist->prp_setUpdater(new NodePointUpdater(this));
}

#include <QApplication>
#include <QDesktopWidget>

void TextBox::openTextEditor(const bool &saveUndoRedo) {
    bool ok;
    QString text =
            QInputDialog::getMultiLineText(mMainWindow, getName() + " text",
                                           "Text:",
                                           mText->getCurrentTextValue(), &ok);
    if(ok) {
        mText->setCurrentTextValue(text, saveUndoRedo);
        callUpdateSchedulers();
    }
}

void TextBox::setFont(const QFont &font, const bool &saveUndoRedo) {
    if(saveUndoRedo) {
//        addUndoRedo(new ChangeFontUndoRedo(this, mFont, font));
    }
    clearAllCache();
    mFont = font;
    scheduleUpdate(Animator::USER_CHANGE);
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

void TextBox::setCurrentTextValue(const QString &text,
                                  const bool &saveUndoRedo) {
    mText->setCurrentTextValue(text, saveUndoRedo);
}

bool TextBox::differenceInEditPathBetweenFrames(
        const int& frame1, const int& frame2) const {
    if(mText->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mLinesDist->prp_differencesBetweenRelFrames(frame1, frame2);
}
