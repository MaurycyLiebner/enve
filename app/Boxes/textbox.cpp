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

    mFillSettings->setPaintType(PaintType::FLATPAINT);
    mFillSettings->setCurrentColor(QColor(0, 0, 0));
    mStrokeSettings->setPaintType(PaintType::NOPAINT);

    mText = SPtrCreate(QStringAnimator)("text");
    ca_prependChildAnimator(mEffectsAnimators.data(), mText);
    mText->prp_setInheritedUpdater(SPtrCreate(NodePointUpdater)(this));

    mLinesDist = SPtrCreate(QrealAnimator)(100, 0, 100, 1, "line dist");
    ca_prependChildAnimator(mEffectsAnimators.data(), mLinesDist);
    mLinesDist->prp_setInheritedUpdater(SPtrCreate(NodePointUpdater)(this));
}

#include <QApplication>
#include <QDesktopWidget>

void TextBox::openTextEditor(QWidget* dialogParent) {
    bool ok;
    const QString text =
            QInputDialog::getMultiLineText(
                dialogParent, getName() + " text",
                "Text:", mText->getCurrentValue(), &ok);
    if(ok) mText->setCurrentValue(text);
}

void TextBox::setFont(const QFont &font) {
    mFont = font;
    setPathsOutdated();
    prp_updateInfluenceRangeAfterChanged();
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

QString TextBox::getCurrentValue() {
    return mText->getCurrentValue();
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
        return 0;
    } else {// if(alignment == Qt::AlignRight) {
        return maxWidth - lineWidth;
    }
}

void TextBox::addActionsToMenu(QMenu * const menu, QWidget* const widgetsParent) {
    menu->addAction("Set Text...", [this, widgetsParent]() {
        openTextEditor(widgetsParent);
    });
}

SkPath TextBox::getPathAtRelFrameF(const qreal &relFrame) {
    QPainterPath qPath = QPainterPath();

    const qreal linesDistAtFrame =
            mLinesDist->getCurrentEffectiveValueAtRelFrame(relFrame)*0.01;
    const QString textAtFrame = mText->getValueAtRelFrame(relFrame);
    const QStringList lines = textAtFrame.split(QRegExp("\n|\r\n|\r"));
    QFontMetricsF fm(mFont);
    qreal yT = 0;
    qreal maxWidth = 0;
    for(const auto& line : lines) {
        const qreal lineWidth = fm.width(line);
        if(lineWidth > maxWidth) maxWidth = lineWidth;
    }
    for(const auto& line : lines) {
        const qreal lineWidth = fm.width(line);
        qPath.addText(textForQPainterPath(mAlignment, lineWidth, maxWidth),
                      yT, mFont, line);
        yT += fm.height()*linesDistAtFrame; // changed distance between lines
    }

    // QRectF boundingRect = qPath.boundingRect();
    // qPath.translate(-boundingRect.center());

    return QPainterPathToSkPath(qPath);
}

void TextBox::setCurrentValue(const QString &text) {
    mText->setCurrentValue(text);
}

bool TextBox::differenceInEditPathBetweenFrames(
        const int& frame1, const int& frame2) const {
    if(mText->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mLinesDist->prp_differencesBetweenRelFrames(frame1, frame2);
}
