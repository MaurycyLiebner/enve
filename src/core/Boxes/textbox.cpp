// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Boxes/textbox.h"
#include <QInputDialog>
#include <QMenu>
#include "canvas.h"
#include "Animators/gradientpoints.h"
#include "Animators/qstringanimator.h"
#include "Animators/rastereffectanimators.h"
#include "typemenu.h"
#include "Animators/transformanimator.h"
#include "Animators/outlinesettingsanimator.h"
#include "textboxrenderdata.h"
#include "pathboxrenderdata.h"

TextBox::TextBox() : PathBox(eBoxType::text) {
    prp_setName("text");

    mFillSettings->setPaintType(PaintType::FLATPAINT);
    mFillSettings->setCurrentColor(QColor(0, 0, 0));
    mStrokeSettings->setPaintType(PaintType::NOPAINT);

    const auto pathsUpdater = [this](const UpdateReason reason) {
        setPathsOutdated(reason);
    };

    mText = enve::make_shared<QStringAnimator>("text");
    ca_prependChild(mRasterEffectsAnimators.data(), mText);
    connect(mText.get(), &Property::prp_currentFrameChanged,
            this, pathsUpdater);

    mSpacingCont = enve::make_shared<StaticComplexAnimator>("spacing");
    mLetterSpacing = enve::make_shared<QrealAnimator>(0, -100, 100, 0.1, "letters");
    mWordSpacing = enve::make_shared<QrealAnimator>(1, -100, 100, 0.1, "words");
    mLineSpacing = enve::make_shared<QrealAnimator>(1, -100, 100, 0.1, "lines");

    mSpacingCont->ca_addChild(mLetterSpacing);
    mSpacingCont->ca_addChild(mWordSpacing);
    mSpacingCont->ca_addChild(mLineSpacing);

    ca_prependChild(mRasterEffectsAnimators.data(), mSpacingCont);

    connect(mLetterSpacing.get(), &Property::prp_currentFrameChanged,
            this, pathsUpdater);
    connect(mWordSpacing.get(), &Property::prp_currentFrameChanged,
            this, pathsUpdater);
    connect(mLineSpacing.get(), &Property::prp_currentFrameChanged,
            this, pathsUpdater);

    mTextEffects = enve::make_shared<TextEffectCollection>();
    ca_prependChild(mRasterEffectsAnimators.data(), mTextEffects);
}

#include <QApplication>
#include <QDesktopWidget>

void TextBox::openTextEditor(QWidget* dialogParent) {
    bool ok;
    const QString text =
            QInputDialog::getMultiLineText(
                dialogParent, prp_getName() + " text",
                "Text:", mText->getCurrentValue(), &ok);
    if(ok) mText->setCurrentValue(text);
}

void TextBox::setTextHAlignment(const Qt::Alignment alignment) {
    mHAlignment = alignment;
    setPathsOutdated(UpdateReason::userChange);
}

void TextBox::setTextVAlignment(const Qt::Alignment alignment) {
    mVAlignment = alignment;
    setPathsOutdated(UpdateReason::userChange);
}

void TextBox::setFont(const QFont &font) {
    mFont = font;
    prp_afterWholeInfluenceRangeChanged();
    setPathsOutdated(UpdateReason::userChange);
}

void TextBox::setSelectedFontSize(const qreal size) {
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

stdsptr<BoxRenderData> TextBox::createRenderData() {
    if(mTextEffects->hasEffects()) {
        return enve::make_shared<TextBoxRenderData>(this);
    } else return PathBox::createRenderData();
}

void TextBox::setupRenderData(const qreal relFrame,
                              BoxRenderData * const data,
                              Canvas * const scene) {
    if(!mTextEffects->hasEffects()) {
        return PathBox::setupRenderData(relFrame, data, scene);
    }
    BoundingBox::setupRenderData(relFrame, data, scene);

    const SkFont font = toSkFont(mFont);
    const QString textAtFrame = mText->getValueAtRelFrame(relFrame);

    const qreal letterSpacing = mLetterSpacing->getEffectiveValue(relFrame);
    const qreal wordSpacing = mWordSpacing->getEffectiveValue(relFrame);
    const qreal lineSpacing = mLineSpacing->getEffectiveValue(relFrame);

    const auto textData = static_cast<TextBoxRenderData*>(data);
    textData->initialize(textAtFrame, font,
                         letterSpacing, wordSpacing, lineSpacing,
                         mHAlignment, mVAlignment, this, scene);
    QList<TextEffect*> textEffects;
    mTextEffects->addEffects(textEffects);
    for(const auto textEffect : textEffects) {
        textEffect->apply(textData);
    }
    textData->queAllLines();

    if(mCurrentPathsOutdated) {
        mEditPathSk = getPathAtRelFrameF(anim_getCurrentRelFrame());
        mPathSk = mEditPathSk;
        mFillPathSk = mEditPathSk;

        mCurrentPathsOutdated = false;
    }
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

void TextBox::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<TextBox>()) return;
    menu->addedActionsForType<TextBox>();
    PathBox::setupCanvasMenu(menu);
    const auto widget = menu->getParentWidget();
    menu->addSeparator();
    PropertyMenu::PlainSelectedOp<TextBox> txtEff = [](TextBox * box) {
        box->mTextEffects->addChild(enve::make_shared<TextEffect>());
    };
    menu->addPlainAction("Add Text Effect", txtEff);
    PropertyMenu::PlainSelectedOp<TextBox> setText = [widget](TextBox * box) {
        box->openTextEditor(widget);
    };
    menu->addPlainAction("Set Text...", setText);
}
#include "include/core/SkFontMetrics.h"
SkPath TextBox::getPathAtRelFrameF(const qreal relFrame) {
    const SkFont font = toSkFont(mFont);
    const qreal fontSize = static_cast<qreal>(font.getSize());
    const QString textAtFrame = mText->getValueAtRelFrame(relFrame);

    const qreal letterSpacing = mLetterSpacing->getEffectiveValue(relFrame);
    const qreal wordSpacing = mWordSpacing->getEffectiveValue(relFrame);
    const qreal lineSpacing = mLineSpacing->getEffectiveValue(relFrame);

    const qreal lineInc = static_cast<qreal>(font.getSpacing())*lineSpacing;

    const QStringList lines = textAtFrame.split(QRegExp("\n|\r\n|\r"));
    qreal maxWidth = 0;
    QList<qreal> lineWidths;
    for(const auto& line : lines) {
        const qreal lineWidth = horizontalAdvance(
                    font, line, letterSpacing, wordSpacing);
        if(lineWidth > maxWidth) maxWidth = lineWidth;
        lineWidths << lineWidth;
    }
    qreal xTranslate;
    if(mHAlignment == Qt::AlignLeft) xTranslate = 0;
    else if(mHAlignment == Qt::AlignRight) xTranslate = -maxWidth;
    else /*if(mHAlignment == Qt::AlignCenter)*/ xTranslate = -0.5*maxWidth;

    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    const qreal height = (lines.count() - 1)*lineInc +
            static_cast<qreal>(metrics.fAscent + metrics.fDescent);
    qreal yTranslate;
    if(mVAlignment == Qt::AlignTop) yTranslate = 0;
    else if(mVAlignment == Qt::AlignBottom) yTranslate = -height;
    else /*if(mVAlignment == Qt::AlignCenter)*/ yTranslate = -0.5*height;

    SkPath result;
    for(int i = 0; i < lines.count(); i++) {
        const auto& line = lines.at(i);
        if(line.isEmpty()) continue;
        const qreal lineWidth = lineWidths.at(i);
        const qreal lineX = textLineX(mHAlignment, lineWidth, maxWidth) + xTranslate;
        const qreal lineY = i*lineInc + yTranslate;
        if(isZero4Dec(letterSpacing) && isOne4Dec(wordSpacing)) {
            SkPath linePath;
            SkTextUtils::GetPath(line.toUtf8().data(),
                                 static_cast<size_t>(line.length()),
                                 SkTextEncoding::kUTF8,
                                 toSkScalar(lineX), toSkScalar(lineY),
                                 font, &linePath);
            result.addPath(linePath);
        } else if(isZero4Dec(letterSpacing)) {
            qreal xPos = lineX;
            const qreal spaceX = horizontalAdvance(font, " ")*wordSpacing;

            const auto wordFinished =
            [&result, &xPos, lineY, &line, &font](const int i0, const int i) {
                const QString wordStr = line.mid(i0, i - i0 + 1);
                SkPath wordPath;
                SkTextUtils::GetPath(wordStr.toUtf8().data(),
                                     static_cast<size_t>(wordStr.length()),
                                     SkTextEncoding::kUTF8,
                                     toSkScalar(xPos), toSkScalar(lineY),
                                     font, &wordPath);
                result.addPath(wordPath);

                xPos += horizontalAdvance(font, wordStr);
            };

            int i0 = 0;
            int nSpaces = 0;
            for(int i = 0; i < line.length(); i++) {
                if(line.at(i) == ' ') {
                    if(nSpaces == 0 && i != 0) wordFinished(i0, i - 1);
                    nSpaces++;
                    i0 = i + 1;
                    xPos += spaceX;
                    continue;
                }
                nSpaces = 0;
            }
            if(i0 < line.length()) wordFinished(i0, line.length() - 1);
        } else {
            qreal xPos = lineX;
            const qreal spaceX = horizontalAdvance(font, " ")*wordSpacing;

            for(int i = 0; i < line.length(); i++) {
                if(line.at(i) == ' ') {
                    xPos += spaceX;
                    continue;
                }
                const QString letter = line.mid(i, 1);
                SkPath letterPath;
                SkTextUtils::GetPath(letter.toUtf8().data(),
                                     static_cast<size_t>(letter.length()),
                                     SkTextEncoding::kUTF8,
                                     toSkScalar(xPos), toSkScalar(lineY),
                                     font, &letterPath);
                result.addPath(letterPath);

                xPos += horizontalAdvance(font, letter) + letterSpacing*fontSize;
            }
        }
    }
    return result;
}

void TextBox::setCurrentValue(const QString &text) {
    mText->setCurrentValue(text);
}

bool TextBox::differenceInEditPathBetweenFrames(
        const int frame1, const int frame2) const {
    if(mText->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mLineSpacing->prp_differencesBetweenRelFrames(frame1, frame2);
}


void TextBox::writeBoundingBox(eWriteStream& dst) {
    PathBox::writeBoundingBox(dst);
    dst.write(&mHAlignment, sizeof(Qt::Alignment));
    dst.write(&mVAlignment, sizeof(Qt::Alignment));
    dst << mFont.pointSizeF();
    dst << mFont.family();
    dst << mFont.styleName();
}

void TextBox::readBoundingBox(eReadStream& src) {
    PathBox::readBoundingBox(src);
    src.read(&mHAlignment, sizeof(Qt::Alignment));
    src.read(&mVAlignment, sizeof(Qt::Alignment));
    qreal fontSize;
    QString fontFamily;
    QString fontStyle;
    src >> fontSize;
    src >> fontFamily;
    src >> fontStyle;
    mFont.setPointSizeF(fontSize);
    mFont.setFamily(fontFamily);
    mFont.setStyleName(fontStyle);
}
