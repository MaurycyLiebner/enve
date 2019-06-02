#include "evfileio.h"
#include <fstream>
#include "Animators/qrealanimator.h"
#include "Animators/randomqrealgenerator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/coloranimator.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Animators/qstringanimator.h"
#include "Animators/transformanimator.h"
#include "Animators/paintsettings.h"
#include "Animators/qrealanimator.h"
#include "Animators/gradient.h"
#include "Properties/comboboxproperty.h"
#include "Properties/intproperty.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/boundingbox.h"
#include "Boxes/pathbox.h"
#include "Boxes/containerbox.h"
#include "Boxes/rectangle.h"
#include "Boxes/circle.h"
#include "Boxes/imagebox.h"
#include "Boxes/videobox.h"
#include "Boxes/textbox.h"
#include "Boxes/particlebox.h"
#include "Boxes/imagesequencebox.h"
#include "Boxes/linkbox.h"
#include "Boxes/paintbox.h"
//#include "GUI/BrushWidgets/brushsettingswidget.h"
#include "canvas.h"
#include "durationrectangle.h"
#include "Animators/gradientpoints.h"
#include "MovablePoints/gradientpoint.h"
#include "Animators/qrealkey.h"
#include "GUI/mainwindow.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include <QMessageBox>
#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"
#include "basicreadwrite.h"
#include "Boxes/internallinkcanvas.h"
#include "Boxes/smartvectorpath.h"
#include "Sound/singlesound.h"

class FileFooter {
public:
    static bool sWrite(QIODevice * const target) {
        return target->write(rcConstChar(sEVFormat), sizeof(char[15])) &&
               target->write(rcConstChar(sAppName), sizeof(char[15])) &&
               target->write(rcConstChar(sAppVersion), sizeof(char[15]));
    }

    static bool sCompatible(QIODevice *target) {
        const qint64 savedPos = target->pos();
        const qint64 pos = target->size() -
                static_cast<qint64>(3*sizeof(char[15]));
        if(!target->seek(pos)) return false;

        char format[15];
        target->read(rcChar(format), sizeof(char[15]));
        if(std::strcmp(format, sEVFormat)) return false;

//        char appVersion[15];
//        target->read(rcChar(appVersion), sizeof(char[15]));

//        char appName[15];
//        target->read(rcChar(appName), sizeof(char[15]));

        if(!target->seek(savedPos))
            RuntimeThrow("Could not restore current position for QIODevice.");
        return true;
    }
private:
    static char sEVFormat[15];
    static char sAppName[15];
    static char sAppVersion[15];
};

char FileFooter::sEVFormat[15] = "enve ev";
char FileFooter::sAppName[15] = "enve";
char FileFooter::sAppVersion[15] = "0.5";

void ComboBoxProperty::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mCurrentValue), sizeof(int));
}

void ComboBoxProperty::readProperty(QIODevice *target) {
    target->read(rcChar(&mCurrentValue), sizeof(int));
}

void Key::writeKey(QIODevice *target) {
    target->write(rcConstChar(&mRelFrame), sizeof(int));
}

void Key::readKey(QIODevice *target) {
    target->read(rcChar(&mRelFrame), sizeof(int));
}

void Animator::writeSelectedKeys(QIODevice* target) {
    const int nKeys = anim_mSelectedKeys.count();
    target->write(rcConstChar(&nKeys), sizeof(int));
    for(const auto& key : anim_mSelectedKeys) {
        key->writeKey(target);
    }
}

void QrealKey::writeKey(QIODevice *target) {
    Key::writeKey(target);
    target->write(rcConstChar(&mValue), sizeof(qreal));

    target->write(rcConstChar(&mStartEnabled), sizeof(bool));
    target->write(rcConstChar(&mStartPt), sizeof(ClampedPoint));

    target->write(rcConstChar(&mEndEnabled), sizeof(bool));
    target->write(rcConstChar(&mEndPt), sizeof(ClampedPoint));
}

void QrealKey::readKey(QIODevice *target) {
    Key::readKey(target);
    target->read(rcChar(&mValue), sizeof(qreal));

    target->read(rcChar(&mStartEnabled), sizeof(bool));
    target->read(rcChar(&mStartPt), sizeof(ClampedPoint));

    target->read(rcChar(&mEndEnabled), sizeof(bool));
    target->read(rcChar(&mEndPt), sizeof(ClampedPoint));
}

void QrealAnimator::writeProperty(QIODevice * const target) const {
    writeKeys(target);
    target->write(rcConstChar(&mCurrentBaseValue), sizeof(qreal));

    const bool hasRandomGenerator = !mRandomGenerator.isNull();
    target->write(rcConstChar(&hasRandomGenerator), sizeof(bool));
    if(hasRandomGenerator) mRandomGenerator->writeProperty(target);
}

stdsptr<Key> QrealAnimator::readKey(QIODevice *target) {
    auto newKey = SPtrCreate(QrealKey)(this);
    newKey->readKey(target);
    return std::move(newKey);
}

void RandomQrealGenerator::writeProperty(QIODevice * const target) const {
    mTime->writeProperty(target);
    mSmoothness->writeProperty(target);
    mMaxDev->writeProperty(target);
    mType->writeProperty(target);
}

void RandomQrealGenerator::readProperty(QIODevice *target) {
    mTime->readProperty(target);
    mSmoothness->readProperty(target);
    mMaxDev->readProperty(target);
    mType->readProperty(target);
}

void QrealAnimator::readProperty(QIODevice *target) {
    readKeys(target);

    qreal val;
    target->read(rcChar(&val), sizeof(qreal));
    setCurrentBaseValue(val);
    bool hasRandomGenerator;
    target->read(rcChar(&hasRandomGenerator), sizeof(bool));
    if(hasRandomGenerator) {
        auto generator = SPtrCreate(RandomQrealGenerator)();
        generator->readProperty(target);
        setGenerator(generator);
    }
}

void QPointFAnimator::writeProperty(QIODevice * const target) const {
    mXAnimator->writeProperty(target);
    mYAnimator->writeProperty(target);
}

void QPointFAnimator::readProperty(QIODevice *target) {
    mXAnimator->readProperty(target);
    mYAnimator->readProperty(target);
}

void ColorAnimator::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mColorMode), sizeof(ColorMode));
    mVal1Animator->writeProperty(target);
    mVal2Animator->writeProperty(target);
    mVal3Animator->writeProperty(target);
    mAlphaAnimator->writeProperty(target);
}

void ColorAnimator::readProperty(QIODevice *target) {
    target->read(rcChar(&mColorMode), sizeof(ColorMode));
    setColorMode(mColorMode);
    mVal1Animator->readProperty(target);
    mVal2Animator->readProperty(target);
    mVal3Animator->readProperty(target);
    mAlphaAnimator->readProperty(target);
}

void PixmapEffect::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mType), sizeof(PixmapEffectType));
    target->write(rcConstChar(&mVisible), sizeof(bool));
}

void PixmapEffect::readProperty(QIODevice *target) {
    target->read(rcChar(&mVisible), sizeof(bool));
}

void BlurEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mBlurRadius->readProperty(target);
}

void BlurEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mBlurRadius->writeProperty(target);
}

void ShadowEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mBlurRadius->readProperty(target);
    mOpacity->readProperty(target);
    mColor->readProperty(target);
    mTranslation->readProperty(target);
}

void ShadowEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mBlurRadius->writeProperty(target);
    mOpacity->writeProperty(target);
    mColor->writeProperty(target);
    mTranslation->writeProperty(target);
}

void DesaturateEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mInfluenceAnimator->readProperty(target);
}

void DesaturateEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mInfluenceAnimator->writeProperty(target);
}

void ColorizeEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mHueAnimator->readProperty(target);
    mSaturationAnimator->readProperty(target);
    mLightnessAnimator->readProperty(target);
    mAlphaAnimator->readProperty(target);
}

void ColorizeEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mHueAnimator->writeProperty(target);
    mSaturationAnimator->writeProperty(target);
    mLightnessAnimator->writeProperty(target);
    mAlphaAnimator->writeProperty(target);
}

void ReplaceColorEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mFromColor->readProperty(target);
    mToColor->readProperty(target);
    mToleranceAnimator->readProperty(target);
    mSmoothnessAnimator->readProperty(target);
}

void ReplaceColorEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mFromColor->writeProperty(target);
    mToColor->writeProperty(target);
    mToleranceAnimator->writeProperty(target);
    mSmoothnessAnimator->writeProperty(target);
}

void ContrastEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mContrastAnimator->readProperty(target);
}

void ContrastEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mContrastAnimator->writeProperty(target);
}

void BrightnessEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mBrightnessAnimator->readProperty(target);
}

void BrightnessEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mBrightnessAnimator->writeProperty(target);
}

void SampledMotionBlurEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mOpacity->readProperty(target);
    mNumberSamples->readProperty(target);
    mFrameStep->readProperty(target);
}

void SampledMotionBlurEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mOpacity->writeProperty(target);
    mNumberSamples->writeProperty(target);
    mFrameStep->writeProperty(target);
}

void EffectAnimators::writeProperty(QIODevice * const target) const {
    const int nEffects = ca_mChildAnimators.count();
    target->write(rcConstChar(&nEffects), sizeof(int));
    for(const auto &effect : ca_mChildAnimators) {
        effect->writeProperty(target);
    }
}

void EffectAnimators::readPixmapEffect(QIODevice *target) {
    PixmapEffectType typeT;
    target->read(rcChar(&typeT), sizeof(PixmapEffectType));
    qsptr<PixmapEffect> effect;
    if(typeT == EFFECT_BLUR) {
        effect = SPtrCreate(BlurEffect)();
    } else if(typeT == EFFECT_SHADOW) {
        effect = SPtrCreate(ShadowEffect)();
    } else if(typeT == EFFECT_DESATURATE) {
        effect = SPtrCreate(DesaturateEffect)();
    } else if(typeT == EFFECT_COLORIZE) {
        effect = SPtrCreate(ColorizeEffect)();
    } else if(typeT == EFFECT_REPLACE_COLOR) {
        effect = SPtrCreate(ReplaceColorEffect)();
    } else if(typeT == EFFECT_BRIGHTNESS) {
        effect = SPtrCreate(BrightnessEffect)();
    } else if(typeT == EFFECT_CONTRAST) {
        effect = SPtrCreate(ContrastEffect)();
    } else if(typeT == EFFECT_MOTION_BLUR) {
        effect = SPtrCreate(SampledMotionBlurEffect)(mParentBox_k);
    } else {
        RuntimeThrow("Invalid pixmap effect type '" +
                     QString::number(typeT) + "'.");
    }
    effect->readProperty(target);
    addEffect(effect);
}

void EffectAnimators::readProperty(QIODevice *target) {
    int nEffects;
    target->read(rcChar(&nEffects), sizeof(int));
    for(int i = 0; i < nEffects; i++) {
        readPixmapEffect(target);
    }
}

void BasicTransformAnimator::writeProperty(QIODevice * const target) const {
    mPosAnimator->writeProperty(target);
    mScaleAnimator->writeProperty(target);
    mRotAnimator->writeProperty(target);
}

void BasicTransformAnimator::readProperty(QIODevice *target) {
    mPosAnimator->readProperty(target);
    mScaleAnimator->readProperty(target);
    mRotAnimator->readProperty(target);
    updateRelativeTransform(Animator::USER_CHANGE);
}

void BoxTransformAnimator::writeProperty(QIODevice * const target) const {
    BasicTransformAnimator::writeProperty(target);
    mShearAnimator->writeProperty(target);
    mOpacityAnimator->writeProperty(target);
    mPivotAnimator->writeProperty(target);
}

void BoxTransformAnimator::readProperty(QIODevice *target) {
    // pivot will be read anyway, so temporarly disable adjusting
    BasicTransformAnimator::readProperty(target);
    mShearAnimator->readProperty(target);
    mOpacityAnimator->readProperty(target);
    mPivotAnimator->readProperty(target);

    updateRelativeTransform(Animator::USER_CHANGE);
}

void GradientPoints::writeProperty(QIODevice * const target) const {
    mStartAnimator->writeProperty(target);
    mEndAnimator->writeProperty(target);
}

void GradientPoints::readProperty(QIODevice *target) {
    mStartAnimator->readProperty(target);
    mEndAnimator->readProperty(target);
}

void Gradient::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mLoadId), sizeof(int));
    const int nColors = mColors.count();
    target->write(rcConstChar(&nColors), sizeof(int));
    for(const auto& color : mColors) {
        color->writeProperty(target);
    }
}

void Gradient::readProperty(QIODevice *target) {
    target->read(rcChar(&mLoadId), sizeof(int));
    int nColors;
    target->read(rcChar(&nColors), sizeof(int));
    for(int i = 0; i < nColors; i++) {
        const auto colorAnim = SPtrCreate(ColorAnimator)();
        colorAnim->readProperty(target);
        addColorToList(colorAnim);
    }
    updateQGradientStops(UpdateReason::USER_CHANGE);
}

void BrushSettings::writeProperty(QIODevice * const target) const {
    mWidthCurve->writeProperty(target);
    mPressureCurve->writeProperty(target);
    mSpacingCurve->writeProperty(target);
    mTimeCurve->writeProperty(target);
    gWrite(target, mBrush ? mBrush->getCollectionName() : "");
    gWrite(target, mBrush ? mBrush->getBrushName() : "");
}

void BrushSettings::readProperty(QIODevice * target) {
    mWidthCurve->readProperty(target);
    mPressureCurve->readProperty(target);
    mSpacingCurve->readProperty(target);
    mTimeCurve->readProperty(target);
    const QString brushCollection = gReadString(target);
    const QString brushName = gReadString(target);
    mBrush = BrushSelectionWidget::sGetBrush(brushCollection, brushName);
}

void OutlineSettingsAnimator::writeProperty(QIODevice * const target) const {
    PaintSettingsAnimator::writeProperty(target);
    mLineWidth->writeProperty(target);
    target->write(rcConstChar(&mCapStyle), sizeof(Qt::PenCapStyle));
    target->write(rcConstChar(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    target->write(rcConstChar(&mOutlineCompositionMode),
                  sizeof(QPainter::CompositionMode));
    mBrushSettings->writeProperty(target);
}

void OutlineSettingsAnimator::readProperty(QIODevice *target) {
    PaintSettingsAnimator::readProperty(target);
    mLineWidth->readProperty(target);
    target->read(rcChar(&mCapStyle), sizeof(Qt::PenCapStyle));
    target->read(rcChar(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    target->read(rcChar(&mOutlineCompositionMode),
                sizeof(QPainter::CompositionMode));
    mBrushSettings->readProperty(target);
}

void PaintSettingsAnimator::writeProperty(QIODevice * const target) const {
    mGradientPoints->writeProperty(target);
    mColor->writeProperty(target);
    target->write(rcConstChar(&mPaintType), sizeof(PaintType));
    target->write(rcConstChar(&mGradientType), sizeof(bool));
    const int gradId = mGradient ? mGradient->getLoadId() : -1;
    target->write(rcConstChar(&gradId), sizeof(int));
}

void PaintSettingsAnimator::readProperty(QIODevice *target) {
    mGradientPoints->readProperty(target);
    mColor->readProperty(target);
    PaintType paintType;
    target->read(rcChar(&paintType), sizeof(PaintType));
    int gradId;
    target->read(rcChar(&mGradientType), sizeof(bool));
    target->read(rcChar(&gradId), sizeof(int));
    if(gradId != -1) {
        mGradient = MainWindow::getInstance()->getLoadedGradientById(gradId);
    }
    setPaintType(paintType);
}

void DurationRectangle::writeDurationRectangle(QIODevice *target) {
    int minFrame = getMinFrame();
    int maxFrame = getMaxFrame();
    int framePos = getFramePos();
    target->write(rcConstChar(&minFrame), sizeof(int));
    target->write(rcConstChar(&maxFrame), sizeof(int));
    target->write(rcConstChar(&framePos), sizeof(int));
}

void DurationRectangle::readDurationRectangle(QIODevice *target) {
    int minFrame;
    int maxFrame;
    int framePos;
    target->read(rcChar(&minFrame), sizeof(int));
    target->read(rcChar(&maxFrame), sizeof(int));
    target->read(rcChar(&framePos), sizeof(int));
    setMinFrame(minFrame);
    setMaxFrame(maxFrame);
    setFramePos(framePos);
}

void FixedLenAnimationRect::writeDurationRectangle(QIODevice *target) {
    DurationRectangle::writeDurationRectangle(target);
    target->write(rcConstChar(&mBoundToAnimation), sizeof(bool));
    target->write(rcConstChar(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->write(rcConstChar(&mMinAnimationFrame), sizeof(int));
    target->write(rcConstChar(&mMaxAnimationFrame), sizeof(int));
}

void FixedLenAnimationRect::readDurationRectangle(QIODevice *target) {
    DurationRectangle::readDurationRectangle(target);
    int minFrame;
    int maxFrame;
    target->read(rcChar(&mBoundToAnimation), sizeof(bool));
    target->read(rcChar(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->read(rcChar(&minFrame), sizeof(int));
    target->read(rcChar(&maxFrame), sizeof(int));
    setMinAnimationFrame(minFrame);
    setMaxAnimationFrame(maxFrame);
}

void BoundingBox::writeBoundingBox(QIODevice * const target) {
    if(mWriteId < 0) assignWriteId();

    target->write(rcConstChar(&mType), sizeof(BoundingBoxType));
    gWrite(target, prp_mName);
    target->write(rcConstChar(&mWriteId), sizeof(int));
    target->write(rcConstChar(&mVisible), sizeof(bool));
    target->write(rcConstChar(&mLocked), sizeof(bool));
    target->write(rcConstChar(&mBlendModeSk), sizeof(SkBlendMode));
    const bool hasDurRect = mDurationRectangle;
    target->write(rcConstChar(&hasDurRect), sizeof(bool));

    if(hasDurRect)
        mDurationRectangle->writeDurationRectangle(target);

    mTransformAnimator->writeProperty(target);
    mEffectsAnimators->writeProperty(target);
}

void BoundingBox::readBoundingBox(QIODevice * const target) {
    gRead(target, prp_mName);
    target->read(rcChar(&mReadId), sizeof(int));
    target->read(rcChar(&mVisible), sizeof(bool));
    target->read(rcChar(&mLocked), sizeof(bool));
    target->read(rcChar(&mBlendModeSk), sizeof(SkBlendMode));
    bool hasDurRect;
    target->read(rcChar(&hasDurRect), sizeof(bool));

    if(hasDurRect) {
        if(!mDurationRectangle) createDurationRectangle();
        mDurationRectangle->readDurationRectangle(target);
        updateAfterDurationRectangleShifted(0);
    }

    mTransformAnimator->readProperty(target);
    mEffectsAnimators->readProperty(target);

    if(hasDurRect) anim_shiftAllKeys(prp_getFrameShift());

    BoundingBox::sAddReadBox(this);
}

void PathEffect::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mPathEffectType), sizeof(PathEffectType));
    target->write(rcConstChar(&mVisible), sizeof(bool));
}

void PathEffect::readProperty(QIODevice *target) {
    target->read(rcChar(&mVisible), sizeof(bool));
}

void DisplacePathEffect::writeProperty(QIODevice * const target) const {
    PathEffect::writeProperty(target);
    mSegLength->writeProperty(target);
    mMaxDev->writeProperty(target);
    mSmoothness->writeProperty(target);
    mLengthBased->writeProperty(target);
    mSeed->writeProperty(target);
}

void DisplacePathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mSegLength->readProperty(target);
    mMaxDev->readProperty(target);
    mSmoothness->readProperty(target);
    mLengthBased->readProperty(target);
    mSeed->readProperty(target);
}

void DuplicatePathEffect::writeProperty(QIODevice * const target) const {
    PathEffect::writeProperty(target);
    mTranslation->writeProperty(target);
}

void DuplicatePathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mTranslation->readProperty(target);
}

void LengthPathEffect::writeProperty(QIODevice * const target) const {
    PathEffect::writeProperty(target);
    mLength->writeProperty(target);
    mReverse->writeProperty(target);
}

void LengthPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mLength->readProperty(target);
    mReverse->readProperty(target);
}

void SolidifyPathEffect::writeProperty(QIODevice * const target) const {
    PathEffect::writeProperty(target);
    mDisplacement->writeProperty(target);
}

void SolidifyPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mDisplacement->readProperty(target);
}

void BoxTargetProperty::writeProperty(QIODevice * const target) const {
    const auto targetBox = mTarget_d.data();
    int targetWriteId = -1;
    int targetDocumentId = -1;

    if(targetBox) {
        if(targetBox->getWriteId() < 0) targetBox->assignWriteId();
        targetWriteId = targetBox->getWriteId();
        targetDocumentId = targetBox->getDocumentId();
    }
    target->write(rcConstChar(&targetWriteId), sizeof(int));
    target->write(rcConstChar(&targetDocumentId), sizeof(int));
}

void BoxTargetProperty::readProperty(QIODevice *target) {
    int targetReadId;
    target->read(rcChar(&targetReadId), sizeof(int));
    int targetDocumentId;
    target->read(rcChar(&targetDocumentId), sizeof(int));
    const auto targetBox = BoundingBox::sGetBoxByReadId(targetReadId);
    if(!targetBox && targetReadId >= 0) {
        QPointer<BoxTargetProperty> thisPtr = this;
        WaitingForBoxLoad::BoxReadFunc readFunc =
        [thisPtr](BoundingBox* box) {
            if(!thisPtr) return;
            thisPtr->setTarget(box);
        };
        WaitingForBoxLoad::BoxNeverReadFunc neverReadFunc =
        [thisPtr, targetDocumentId]() {
            if(!thisPtr) return;
            const auto box = BoundingBox::sGetBoxByDocumentId(targetDocumentId);
            thisPtr->setTarget(box);
        };
        const auto func = WaitingForBoxLoad(targetReadId,
                                            readFunc, neverReadFunc);
        BoundingBox::sAddWaitingForBoxLoad(func);
    } else {
        setTarget(targetBox);
    }
}

void OperationPathEffect::writeProperty(QIODevice * const target) const {
    PathEffect::writeProperty(target);
    mBoxTarget->writeProperty(target);
}

void OperationPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mBoxTarget->readProperty(target);
}

void PathEffectAnimators::writeProperty(QIODevice * const target) const {   
    const int nEffects = ca_mChildAnimators.count();
    target->write(rcConstChar(&nEffects), sizeof(int));
    for(const auto &effect : ca_mChildAnimators) {
        effect->writeProperty(target);
    }
}

void PathEffectAnimators::readPathEffect(QIODevice *target) {
    PathEffectType typeT;
    target->read(rcChar(&typeT), sizeof(PathEffectType));
    qsptr<PathEffect> pathEffect;
    if(typeT == DISPLACE_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(DisplacePathEffect)(mIsOutline);
    } else if(typeT == DUPLICATE_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(DuplicatePathEffect)(mIsOutline);
    } else if(typeT == OPERATION_PATH_EFFECT) {
        pathEffect = SPtrCreate(OperationPathEffect)(mIsOutline);
    } else if(typeT == LENGTH_PATH_EFFECT) {
        //pathEffect = SPtrCreate(LengthPathEffect)(mIsOutline);
        const auto subPathEffect = SPtrCreate(SubPathEffect)(mIsOutline);
        subPathEffect->readLengthEffect(target);
        addEffect(subPathEffect);
        return;
    } else if(typeT == SOLIDIFY_PATH_EFFECT) {
        pathEffect = SPtrCreate(SolidifyPathEffect)(mIsOutline);
    } else if(typeT == SUM_PATH_EFFECT) {
        pathEffect = SPtrCreate(SumPathEffect)(mIsOutline);
    } else if(typeT == SUB_PATH_EFFECT) {
        pathEffect = SPtrCreate(SubPathEffect)(mIsOutline);
    } else if(typeT == LINES_PATH_EFFECT) {
        pathEffect = SPtrCreate(LinesPathEffect)(mIsOutline);
    } else if(typeT == ZIGZAG_PATH_EFFECT) {
        pathEffect = SPtrCreate(ZigZagPathEffect)(mIsOutline);
    } else if(typeT == SPATIAL_DISPLACE_PATH_EFFECT) {
        pathEffect = SPtrCreate(SpatialDisplacePathEffect)(mIsOutline);
    } else {
        RuntimeThrow("Invalid path effect type '" +
                     QString::number(typeT) + "'.");
    }
    pathEffect->readProperty(target);
    addEffect(pathEffect);
}

void PathEffectAnimators::readProperty(QIODevice *target) {
    int nEffects;
    target->read(rcChar(&nEffects), sizeof(int));
    for(int i = 0; i < nEffects; i++) {
        readPathEffect(target);
    }
}

void PathBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    mPathEffectsAnimators->writeProperty(target);
    mFillPathEffectsAnimators->writeProperty(target);
    mOutlinePathEffectsAnimators->writeProperty(target);
    mFillGradientPoints->writeProperty(target);
    mStrokeGradientPoints->writeProperty(target);
    mFillSettings->writeProperty(target);
    mStrokeSettings->writeProperty(target);
}

void PathBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    mPathEffectsAnimators->readProperty(target);
    mFillPathEffectsAnimators->readProperty(target);
    mOutlinePathEffectsAnimators->readProperty(target);
    mFillGradientPoints->readProperty(target);
    mStrokeGradientPoints->readProperty(target);
    mFillSettings->readProperty(target);
    mStrokeSettings->readProperty(target);
}
#include "Animators/SmartPath/smartpathcollection.h"
void SmartVectorPath::writeBoundingBox(QIODevice * const target) {
    PathBox::writeBoundingBox(target);
    mPathAnimator->writeProperty(target);
}
void SmartVectorPath::readBoundingBox(QIODevice * const target) {
    PathBox::readBoundingBox(target);
    mPathAnimator->readProperty(target);
}

void ParticleEmitter::writeProperty(QIODevice * const target) const {
    mColorAnimator->writeProperty(target);
    mPos->writeProperty(target);
    mWidth->writeProperty(target);
    mSrcVelInfl->writeProperty(target);
    mIniVelocity->writeProperty(target);
    mIniVelocityVar->writeProperty(target);
    mIniVelocityAngle->writeProperty(target);
    mIniVelocityAngleVar->writeProperty(target);
    mAcceleration->writeProperty(target);
    mParticlesPerSecond->writeProperty(target);
    mParticlesFrameLifetime->writeProperty(target);
    mVelocityRandomVar->writeProperty(target);
    mVelocityRandomVarPeriod->writeProperty(target);
    mParticleSize->writeProperty(target);
    mParticleSizeVar->writeProperty(target);
    mParticleLength->writeProperty(target);
    mParticlesDecayFrames->writeProperty(target);
    mParticlesSizeDecay->writeProperty(target);
    mParticlesOpacityDecay->writeProperty(target);
}

void ParticleEmitter::readProperty(QIODevice *target) {
    mColorAnimator->readProperty(target);
    mPos->readProperty(target);
    mWidth->readProperty(target);
    mSrcVelInfl->readProperty(target);
    mIniVelocity->readProperty(target);
    mIniVelocityVar->readProperty(target);
    mIniVelocityAngle->readProperty(target);
    mIniVelocityAngleVar->readProperty(target);
    mAcceleration->readProperty(target);
    mParticlesPerSecond->readProperty(target);
    mParticlesFrameLifetime->readProperty(target);
    mVelocityRandomVar->readProperty(target);
    mVelocityRandomVarPeriod->readProperty(target);
    mParticleSize->readProperty(target);
    mParticleSizeVar->readProperty(target);
    mParticleLength->readProperty(target);
    mParticlesDecayFrames->readProperty(target);
    mParticlesSizeDecay->readProperty(target);
    mParticlesOpacityDecay->readProperty(target);
}

void ParticleBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    mTopLeftAnimator->writeProperty(target);
    mBottomRightAnimator->writeProperty(target);
    int nEmitters = mEmitters.count();
    target->write(rcConstChar(&nEmitters), sizeof(int));
    for(const auto& emitter : mEmitters) {
        emitter->writeProperty(target);
    }
}

void ParticleBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    mTopLeftAnimator->readProperty(target);
    mBottomRightAnimator->readProperty(target);
    int nEmitters;
    target->read(rcChar(&nEmitters), sizeof(int));
    for(int i = 0; i < nEmitters; i++) {
        auto emitter = SPtrCreate(ParticleEmitter)(this);
        emitter->readProperty(target);
        addEmitter(emitter);
    }
}

void ImageBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    gWrite(target, mImageFilePath);
}

void ImageBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    gRead(target, path);
    setFilePath(path);
}

void Circle::writeBoundingBox(QIODevice * const target) {
    PathBox::writeBoundingBox(target);
    mHorizontalRadiusAnimator->writeProperty(target);
    mVerticalRadiusAnimator->writeProperty(target);
}

void Circle::readBoundingBox(QIODevice * const target) {
    PathBox::readBoundingBox(target);
    mHorizontalRadiusAnimator->readProperty(target);
    mVerticalRadiusAnimator->readProperty(target);
}

void Rectangle::writeBoundingBox(QIODevice * const target) {
    PathBox::writeBoundingBox(target);
    mRadiusAnimator->writeProperty(target);
    mTopLeftAnimator->writeProperty(target);
    mBottomRightAnimator->writeProperty(target);
}

void Rectangle::readBoundingBox(QIODevice * const target) {
    PathBox::readBoundingBox(target);
    mRadiusAnimator->readProperty(target);
    mTopLeftAnimator->readProperty(target);
    mBottomRightAnimator->readProperty(target);
}

void VideoBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    gWrite(target, mSrcFilePath);
    const bool hasSound = mSound;
    gWrite(target, hasSound);
    if(hasSound) mSound->writeProperty(target);
}

void VideoBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    gRead(target, path);
    setFilePath(path);
    const bool hasSound = gReadBool(target);
    if(hasSound) {
        if(!mSound) mSound = SPtrCreate(SingleSound)();
        mSound->readProperty(target);
    }
}

void PaintBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
}

void PaintBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
}

void AnimationBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    target->write(rcConstChar(&mFrameRemappingEnabled), sizeof(bool));
    mFrameAnimator->writeProperty(target);
}

void AnimationBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    bool frameRemapping;
    target->read(rcChar(&frameRemapping), sizeof(bool));
    mFrameAnimator->readProperty(target);
    if(frameRemapping) enableFrameRemapping();
    else disableFrameRemapping();
}

void ImageSequenceBox::writeBoundingBox(QIODevice * const target) {
    AnimationBox::writeBoundingBox(target);
    int nFrames = mListOfFrames.count();
    target->write(rcConstChar(&nFrames), sizeof(int));
    for(const QString &frame : mListOfFrames) {
        gWrite(target, frame);
    }
}

void ImageSequenceBox::readBoundingBox(QIODevice * const target) {
    AnimationBox::readBoundingBox(target);
    int nFrames;
    target->read(rcChar(&nFrames), sizeof(int));
    QStringList frames;
    for(int i = 0; i < nFrames; i++) {
        frames << gReadString(target);
    }
    setListOfFrames(frames);
}

void TextBox::writeBoundingBox(QIODevice * const target) {
    PathBox::writeBoundingBox(target);
    mText->writeProperty(target);
    target->write(rcConstChar(&mAlignment), sizeof(Qt::Alignment));
    const qreal fontSize = mFont.pointSizeF();
    const QString fontFamily = mFont.family();
    const QString fontStyle = mFont.styleName();
    target->write(rcConstChar(&fontSize), sizeof(qreal));
    gWrite(target, fontFamily);
    gWrite(target, fontStyle);
}

void TextBox::readBoundingBox(QIODevice * const target) {
    PathBox::readBoundingBox(target);
    mText->readProperty(target);
    target->read(rcChar(&mAlignment), sizeof(Qt::Alignment));
    qreal fontSize;
    QString fontFamily;
    QString fontStyle;
    target->read(rcChar(&fontSize), sizeof(qreal));
    gRead(target, fontFamily);
    gRead(target, fontStyle);
    mFont.setPointSizeF(fontSize);
    mFont.setFamily(fontFamily);
    mFont.setStyleName(fontStyle);
}

#include "Boxes/containerbox.h"
void ContainerBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    mPathEffectsAnimators->writeProperty(target);
    mFillPathEffectsAnimators->writeProperty(target);
    mOutlinePathEffectsAnimators->writeProperty(target);
    int nChildBoxes = mContainedBoxes.count();
    target->write(rcConstChar(&nChildBoxes), sizeof(int));
    for(const auto &child : mContainedBoxes) {
        child->writeBoundingBox(target);
    }
}
#include "Boxes/containerbox.h"
void ContainerBox::readChildBoxes(QIODevice *target) {
    int nChildBoxes;
    target->read(rcChar(&nChildBoxes), sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        qsptr<BoundingBox> box;
        BoundingBoxType boxType;
        target->read(rcChar(&boxType), sizeof(BoundingBoxType));
        if(boxType == TYPE_VECTOR_PATH) {
            box = SPtrCreate(SmartVectorPath)();
        } else if(boxType == TYPE_IMAGE) {
            box = SPtrCreate(ImageBox)();
        } else if(boxType == TYPE_TEXT) {
            box = SPtrCreate(TextBox)();
        } else if(boxType == TYPE_VIDEO) {
            box = SPtrCreate(VideoBox)();
        } else if(boxType == TYPE_PARTICLES) {
            box = SPtrCreate(ParticleBox)();
        } else if(boxType == TYPE_RECTANGLE) {
            box = SPtrCreate(Rectangle)();
        } else if(boxType == TYPE_CIRCLE) {
            box = SPtrCreate(Circle)();
        } else if(boxType == TYPE_LAYER) {
            box = SPtrCreate(ContainerBox)(TYPE_LAYER);
        } else if(boxType == TYPE_GROUP) {
            box = SPtrCreate(ContainerBox)(TYPE_GROUP);
        } else if(boxType == TYPE_PAINT) {
            box = SPtrCreate(PaintBox)();
        } else if(boxType == TYPE_IMAGESQUENCE) {
            box = SPtrCreate(ImageSequenceBox)();
        } else if(boxType == TYPE_INTERNAL_LINK) {
            box = SPtrCreate(InternalLinkBox)(nullptr);
        } else if(boxType == TYPE_INTERNAL_LINK_GROUP) {
            box = SPtrCreate(InternalLinkGroupBox)(nullptr);
        } else if(boxType == TYPE_EXTERNAL_LINK) {
            box = SPtrCreate(ExternalLinkBox)();
        } else if(boxType == TYPE_INTERNAL_LINK_CANVAS) {
            box = SPtrCreate(InternalLinkCanvas)(nullptr);
        } else {
            RuntimeThrow("Invalid box type '" + std::to_string(boxType) + "'");
        }

        box->readBoundingBox(target);
        addContainedBox(box);
    }
}

void ContainerBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    mPathEffectsAnimators->readProperty(target);
    mFillPathEffectsAnimators->readProperty(target);
    mOutlinePathEffectsAnimators->readProperty(target);
    readChildBoxes(target);
}

void Canvas::writeBoundingBox(QIODevice * const target) {
    ContainerBox::writeBoundingBox(target);
    const int currFrame = getCurrentFrame();
    target->write(rcConstChar(&currFrame), sizeof(int));
    target->write(rcConstChar(&mClipToCanvasSize), sizeof(bool));
    target->write(rcConstChar(&mWidth), sizeof(int));
    target->write(rcConstChar(&mHeight), sizeof(int));
    target->write(rcConstChar(&mFps), sizeof(qreal));
    mBackgroundColor->writeProperty(target);
    target->write(rcConstChar(&mMaxFrame), sizeof(int));
    target->write(rcConstChar(&mCanvasTransform),
                  sizeof(QMatrix));
    mSoundComposition->writeSounds(target);
}

void Canvas::readBoundingBox(QIODevice * const target) {
    target->read(rcChar(&mType), sizeof(BoundingBoxType));
    ContainerBox::readBoundingBox(target);
    int currFrame;
    target->read(rcChar(&currFrame), sizeof(int));
    target->read(rcChar(&mClipToCanvasSize), sizeof(bool));
    target->read(rcChar(&mWidth), sizeof(int));
    target->read(rcChar(&mHeight), sizeof(int));
    target->read(rcChar(&mFps), sizeof(qreal));
    mBackgroundColor->readProperty(target);
    target->read(rcChar(&mMaxFrame), sizeof(int));
    target->read(rcChar(&mCanvasTransform), sizeof(QMatrix));
    mVisibleHeight = mCanvasTransform.m22()*mHeight;
    mVisibleWidth = mCanvasTransform.m11()*mWidth;
    anim_setAbsFrame(currFrame);
    mSoundComposition->readSounds(target);
}

void GradientWidget::writeGradients(QIODevice *target) {
    const int nGradients = mGradients.count();
    target->write(rcConstChar(&nGradients), sizeof(int));
    for(const auto &gradient : mGradients) {
        gradient->writeProperty(target);
    }
}

void GradientWidget::readGradients(QIODevice *target) {
    int nGradients;
    target->read(rcChar(&nGradients), sizeof(int));
    for(int i = 0; i < nGradients; i++) {
        auto gradient = SPtrCreate(Gradient)();
        gradient->readProperty(target);
        addGradientToList(gradient);
        MainWindow::getInstance()->addLoadedGradient(gradient.get());
    }
}

QColor readQColor(QIODevice *read) {
    QColor qcolT;
    read->read(rcChar(&qcolT), sizeof(QColor));
    return qcolT;
}

void writeQColor(const QColor& qcol, QIODevice *write) {
    write->write(rcConstChar(&qcol), sizeof(QColor));
}

void CanvasWindow::writeCanvases(QIODevice *target) {
    const int nCanvases = mCanvasList.count();
    target->write(rcConstChar(&nCanvases), sizeof(int));
    int currentCanvasId = -1;
    for(const auto &canvas : mCanvasList) {
        canvas->writeBoundingBox(target);
        if(canvas.get() == mCurrentCanvas) {
            currentCanvasId = mCurrentCanvas->getWriteId();
        }
    }
    target->write(rcConstChar(&currentCanvasId), sizeof(int));
}

void CanvasWindow::readCanvases(QIODevice *target) {
    int nCanvases;
    target->read(rcChar(&nCanvases), sizeof(int));
    for(int i = 0; i < nCanvases; i++) {
        auto canvas = SPtrCreate(Canvas)(this);
        canvas->readBoundingBox(target);
        MainWindow::getInstance()->addCanvas(canvas);
    }
    int currentCanvasId;
    target->read(rcChar(&currentCanvasId), sizeof(int));
    auto currentCanvas = BoundingBox::sGetBoxByReadId(currentCanvasId);
    setCurrentCanvas(GetAsPtr(currentCanvas, Canvas));
}

void MainWindow::loadEVFile(const QString &path) {
    QFile target(path);
    if(!target.exists()) RuntimeThrow("File does not exist " + path);
    if(!target.open(QIODevice::ReadOnly))
        RuntimeThrow("Could not open file " + path);
    if(!FileFooter::sCompatible(&target)) {
        target.close();
        RuntimeThrow("File incompatible or incomplete " + path);
    }
    auto gradientWidget = mFillStrokeSettings->getGradientWidget();
    gradientWidget->readGradients(&target);
    mCanvasWindow->readCanvases(&target);

    clearLoadedGradientsList();
    gradientWidget->clearGradientsLoadIds();
    BoundingBox::sClearReadBoxes();

    target.close();
}

void MainWindow::saveToFile(const QString &path) {
    QFile target(path);
    if(target.exists()) target.remove();

    if(target.open(QIODevice::WriteOnly)) {
        auto gradientWidget = mFillStrokeSettings->getGradientWidget();
        gradientWidget->setGradientLoadIds();
        gradientWidget->writeGradients(&target);
        mCanvasWindow->writeCanvases(&target);

        clearLoadedGradientsList();
        gradientWidget->clearGradientsLoadIds();

        FileFooter::sWrite(&target);

        target.close();
    } else {
        RuntimeThrow("Could not open file for writing " + path + ".");
    }

    BoundingBox::sClearWriteBoxes();
    addRecentFile(path);
}
