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

qreal textLineX(const Qt::Alignment &alignment,
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
    const QString textAtFrame = mText->getValueAtRelFrame(relFrame);
    const qreal linesDistAtFrame =
            mLinesDist->getEffectiveValueAtRelFrame(relFrame)*0.01;
    const QStringList lines = textAtFrame.split(QRegExp("\n|\r\n|\r"));
    QFontMetricsF fm(mFont);
    qreal maxWidth = 0;
    for(const auto& line : lines) {
        const qreal lineWidth = fm.width(line);
        if(lineWidth > maxWidth) maxWidth = lineWidth;
    }

    SkFont font;
    font.setTypeface(SkTypeface::MakeFromName(
                     mFont.family().toStdString().c_str(),
                     SkFontStyle::Normal()));
    font.setSize(toSkScalar(mFont.pointSizeF()));
    SkPath result;
    //QPainterPath result;
    for(int i = 0; i < lines.count(); i++) {
        const auto& line = lines.at(i);
        if(line.isEmpty()) continue;
        const qreal lineWidth = fm.width(line);
        SkPath linePath;
        const qreal lineX = textLineX(mAlignment, lineWidth, maxWidth);
        const qreal lineY = i*fm.height()*linesDistAtFrame;
        const auto lineStd = line.toStdString();
        const auto lineCStr = lineStd.c_str();
        SkTextUtils::GetPath(lineCStr,
                             static_cast<size_t>(line.length()),
                             SkTextEncoding::kUTF8,
                             toSkScalar(lineX), toSkScalar(lineY),
                             font, &linePath);
        result.addPath(linePath);
        //result.addText(lineX, lineY, mFont, line);
    }
    //return toSkPath(result);
    return result;
}

void TextBox::setCurrentValue(const QString &text) {
    mText->setCurrentValue(text);
}

bool TextBox::differenceInEditPathBetweenFrames(
        const int& frame1, const int& frame2) const {
    if(mText->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mLinesDist->prp_differencesBetweenRelFrames(frame1, frame2);
}
