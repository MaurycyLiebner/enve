#include "texteffectcaller.h"

#include "texteffect.h"
#include "Boxes/textboxrenderdata.h"


//void TextEffectCaller::applyToLetter(PathBoxRenderData * const letterData,
//                               const qreal influence) const {
//    if(isZero4Dec(influence)) return;
//    const qreal relFrame = letterData->fRelFrame;
//    const auto transform = mTransform->getPartialRelTransform(relFrame, influence);
//    letterData->fRelTransform *= transform;
//    letterData->fTotalTransform *= transform;
//}

//void TextEffectCaller::apply(TextBoxRenderData * const textData) {
//    switch(fTarget) {
//    case TextEffectTarget::letters: {
//        const auto infls = influenceGuide(textData->fLetters.count());
//        int i = 0;
//        for(const auto& letter : textData->fLetters) {
//            applyToLetter(letter.get(), infls.getValue(i++));
//        }
//    } break;
//    case TextEffectTarget::words: {
////        const auto infls = influenceGuide(textData->fWords.count(),
////                                          textData->fRelFrame);
////        int i = 0;
////        for(const auto& word : textData->fWords) {
////            applyToWord(word.get(), infls.getValue(i++));
////        }
//    } break;
//    case TextEffectTarget::lines: {
////        const auto infls = influenceGuide(textData->fLines.count(),
////                                          textData->fRelFrame);
////        int i = 0;
////        for(const auto& line : textData->fLines) {
////            applyToLine(line.get(), infls.getValue(i++));
////        }
//    } break;
//    }
//}

//QrealSnapshot TextEffectCaller::influenceGuide(const int nTargets) {
//    QrealSnapshot result(fAmpl, 1, fAmpl);

//    const qreal first = fCenter - qCeil(fCenter);
//    const qreal last = first + nTargets + 2*fFrequency;
//    bool up = qRound((fCenter - first)/fFrequency) % 2;
//    for(qreal i = first ; i < last; i += fFrequency) {
//        qreal baseInfl;
//        if(isZero4Dec(fDiminish)) {
//            baseInfl = 0;
//        } else {
//            qreal val;
//            if(fDiminish > 0) val = 1 - qAbs(i - fCenter)/fDiminish;
//            else val = -qAbs(i - fCenter)/fDiminish;
//            baseInfl = qBound(0., val, 1.);
//            if(qAbs(fDiminish) < 1) baseInfl *= qAbs(fDiminish);
//        }

//        const qreal freqInfl = up ? 1 : 0;
//        up = !up;

//        const qreal infl = fMinInfl + (baseInfl*freqInfl)*fRemInfl;
//        result.appendKey(i, infl, i, infl, i, infl);
//    }
//    return result;
//}
