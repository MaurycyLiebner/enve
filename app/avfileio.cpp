#include "avfileio.h"
#include <fstream>
#include "Animators/PathAnimators/pathkey.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/randomqrealgenerator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/coloranimator.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Animators/qstringanimator.h"
#include "Animators/transformanimator.h"
#include "Animators/paintsettings.h"
#include "Animators/pathanimator.h"
#include "Animators/gradient.h"
#include "Properties/comboboxproperty.h"
#include "Properties/intproperty.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/boundingbox.h"
#include "Boxes/pathbox.h"
#include "Boxes/boxesgroup.h"
#include "Boxes/vectorpath.h"
#include "Boxes/rectangle.h"
#include "Boxes/circle.h"
#include "Boxes/imagebox.h"
#include "Boxes/videobox.h"
#include "Boxes/textbox.h"
#include "Boxes/particlebox.h"
#include "Boxes/imagesequencebox.h"
#include "Boxes/linkbox.h"
#include "Boxes/paintbox.h"
#include "Paint/surface.h"
#include "Paint/animatedsurface.h"
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
#define FORMAT_STR "AniVect av"
#define CREATOR_VERSION "0.1a"
#define CREATOR_APPLICATION "AniVect"

struct FileFooter {
    enum CompatybilityMode {
        Compatible,
        NonCompatible
    };

    const char formatStr[15] = FORMAT_STR;
    const char creatorVersion[15] = CREATOR_VERSION;
    const char creatorApplication[15] = CREATOR_APPLICATION;

    CompatybilityMode combatybilityMode() const {
        if(!std::strcmp(formatStr, FORMAT_STR)) {
            return Compatible;
        }
        return NonCompatible;
    }

    void write(QIODevice * const target) const {
        target->write(rcConstChar(this), sizeof(FileFooter));
    }

    void read(QIODevice *target) {
        const qint64 pos = target->size() - static_cast<qint64>(sizeof(FileFooter));
        target->seek(pos);
        target->read(rcChar(this), sizeof(FileFooter));
        target->seek(0);
    }
};

void BoolProperty::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mValue), sizeof(bool));
}

void BoolProperty::readProperty(QIODevice *target) {
    target->read(rcChar(&mValue), sizeof(bool));
}

void BoolPropertyContainer::writeProperty(QIODevice * const target) const {
    ComplexAnimator::writeProperty(target);
    target->write(rcConstChar(&mValue), sizeof(bool));
}

void BoolPropertyContainer::readProperty(QIODevice *target) {
    ComplexAnimator::readProperty(target);
    bool value;
    target->read(rcChar(&value), sizeof(bool));
    setValue(value);
}


void ComboBoxProperty::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mCurrentValue), sizeof(int));
}

void ComboBoxProperty::readProperty(QIODevice *target) {
    target->read(rcChar(&mCurrentValue), sizeof(int));
}

void IntProperty::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mMinValue), sizeof(int));
    target->write(rcConstChar(&mMaxValue), sizeof(int));
    target->write(rcConstChar(&mValue), sizeof(int));
}

void IntProperty::readProperty(QIODevice *target) {
    target->read(rcChar(&mMinValue), sizeof(int));
    target->read(rcChar(&mMaxValue), sizeof(int));
    target->read(rcChar(&mValue), sizeof(int));
}

void Key::writeKey(QIODevice *target) {
    target->write(rcConstChar(&mRelFrame), sizeof(int));
}

void Key::readKey(QIODevice *target) {
    target->read(rcChar(&mRelFrame), sizeof(int));
}

void PathContainer::writePathContainer(QIODevice * const target) const {
    const int nPts = mElementsPos.count();
    target->write(rcConstChar(&nPts), sizeof(int)); // number pts
    for(const SkPoint &pos : mElementsPos) {
        SkScalar xT, yT;
        xT = pos.x(); yT = pos.y();
        target->write(rcConstChar(&xT), sizeof(SkScalar));
        target->write(rcConstChar(&yT), sizeof(SkScalar));
    }
    target->write(rcConstChar(&mPathClosed), sizeof(bool));
}

void PathContainer::readPathContainer(QIODevice *target) {
    int nPts;
    target->read(rcChar(&nPts), sizeof(int));
    for(int i = 0; i < nPts; i++) {
        SkScalar xT, yT;
        target->read(rcChar(&xT), sizeof(SkScalar));
        target->read(rcChar(&yT), sizeof(SkScalar));
        mElementsPos.append(SkPoint::Make(xT, yT));
    }
    target->read(rcChar(&mPathClosed), sizeof(bool));
    mPathUpdateNeeded = true;
}

void PathKey::writeKey(QIODevice *target) {
    Key::writeKey(target);
    writePathContainer(target);
}

void PathKey::readKey(QIODevice *target) {
    Key::readKey(target);
    readPathContainer(target);
}

void VectorPathAnimator::writeProperty(QIODevice * const target) const {
    const int nNodes = mNodeSettings.count();
    target->write(rcConstChar(&nNodes), sizeof(int));
    for(const auto &nodeSettings : mNodeSettings) {
        nodeSettings->write(target);
    }

    const int nKeys = anim_mKeys.count();
    target->write(rcConstChar(&nKeys), sizeof(int));
    for(const auto &key : anim_mKeys) {
        key->writeKey(target);
    }

    writePathContainer(target);
}

stdsptr<Key> VectorPathAnimator::readKey(QIODevice *target) {
    const auto newKey = SPtrCreate(PathKey)(this);
    newKey->readKey(target);
    return std::move(newKey);
}

void NodeSettings::write(QIODevice* target) {
    target->write(rcConstChar(&ctrlsMode),
                 sizeof(CtrlsMode));
    target->write(rcConstChar(&startEnabled),
                 sizeof(bool));
    target->write(rcConstChar(&endEnabled),
                 sizeof(bool));
}

void NodeSettings::read(QIODevice* target) {
    target->read(rcChar(&ctrlsMode), sizeof(CtrlsMode));
    target->read(rcChar(&startEnabled), sizeof(bool));
    target->read(rcChar(&endEnabled), sizeof(bool));
}

void VectorPathAnimator::readProperty(QIODevice *target) {
    int nNodes;
    target->read(rcChar(&nNodes), sizeof(int));
    for(int i = 0; i < nNodes; i++) {
        auto nodeSettings = SPtrCreate(NodeSettings)();
        nodeSettings->read(target);
        insertNodeSettingsForNodeId(i, nodeSettings);
    }

    int nKeys;
    target->read(rcChar(&nKeys), sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        anim_appendKey(readKey(target));
    }

    readPathContainer(target);
    updateNodePointsFromElements();
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
    target->write(rcConstChar(&mStartFrame), sizeof(qreal));
    target->write(rcConstChar(&mStartValue), sizeof(qreal));

    target->write(rcConstChar(&mEndEnabled), sizeof(bool));
    target->write(rcConstChar(&mEndFrame), sizeof(qreal));
    target->write(rcConstChar(&mEndValue), sizeof(qreal));
}

void QrealKey::readKey(QIODevice *target) {
    Key::readKey(target);
    target->read(rcChar(&mValue), sizeof(qreal));

    target->read(rcChar(&mStartEnabled), sizeof(bool));
    target->read(rcChar(&mStartFrame), sizeof(qreal));
    target->read(rcChar(&mStartValue), sizeof(qreal));

    target->read(rcChar(&mEndEnabled), sizeof(bool));
    target->read(rcChar(&mEndFrame), sizeof(qreal));
    target->read(rcChar(&mEndValue), sizeof(qreal));
}

void QrealAnimator::writeProperty(QIODevice * const target) const {
    const int nKeys = anim_mKeys.count();
    target->write(rcConstChar(&nKeys), sizeof(int));
    for(const auto &key : anim_mKeys) {
        key->writeKey(target);
    }

    target->write(rcConstChar(&mCurrentValue), sizeof(qreal));

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
    mPeriod->writeProperty(target);
    mSmoothness->writeProperty(target);
    mMaxDev->writeProperty(target);
    mType->writeProperty(target);
}

void RandomQrealGenerator::readProperty(QIODevice *target) {
    mPeriod->readProperty(target);
    mSmoothness->readProperty(target);
    mMaxDev->readProperty(target);
    mType->readProperty(target);
    generateData();
}

void QrealAnimator::readProperty(QIODevice *target) {
    int nKeys;
    target->read(rcChar(&nKeys), sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        anim_appendKey(readKey(target));
    }

    qreal val;
    target->read(rcChar(&val), sizeof(qreal));
    qra_setCurrentValue(val);
    bool hasRandomGenerator;
    target->read(rcChar(&hasRandomGenerator), sizeof(bool));
    if(hasRandomGenerator) {
        auto generator = SPtrCreate(RandomQrealGenerator)(0, 9999);
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
    mHighQuality->readProperty(target);
    mBlurRadius->readProperty(target);
}

void BlurEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mHighQuality->writeProperty(target);
    mBlurRadius->writeProperty(target);
}

void ShadowEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mHighQuality->readProperty(target);
    mBlurRadius->readProperty(target);
    mOpacity->readProperty(target);
    mColor->readProperty(target);
    mTranslation->readProperty(target);
}

void ShadowEffect::writeProperty(QIODevice * const target) const {
    PixmapEffect::writeProperty(target);
    mHighQuality->writeProperty(target);
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
        QString errMsg = "Invalid pixmap effect type '" +
                QString::number(typeT) + "'.";
        RuntimeThrow(errMsg.toStdString());
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
    mOpacityAnimator->writeProperty(target);
    mPivotAnimator->writeProperty(target);
}

void BoxTransformAnimator::readProperty(QIODevice *target) {
    BasicTransformAnimator::readProperty(target);
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
        auto colorAnim = SPtrCreate(ColorAnimator)();
        colorAnim->readProperty(target);
        addColorToList(colorAnim);
    }
    updateQGradientStops(UpdateReason::USER_CHANGE);
}

void BrushSettings::writeProperty(QIODevice * const target) const {
    mWidthCurve->writeProperty(target);
    mPressureCurve->writeProperty(target);
    mTimeCurve->writeProperty(target);
    gWrite(target, mBrush ? mBrush->getCollectionName() : "");
    gWrite(target, mBrush ? mBrush->getBrushName() : "");
}

void BrushSettings::readProperty(QIODevice * target) {
    mWidthCurve->readProperty(target);
    mPressureCurve->readProperty(target);
    mTimeCurve->readProperty(target);
    QString brushCollection;
    gRead(target, brushCollection);
    QString brushName;
    gRead(target, brushName);
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
    target->read(rcChar(&mPaintType), sizeof(PaintType));
    int gradId;
    target->read(rcChar(&mGradientType), sizeof(bool));
    target->read(rcChar(&gradId), sizeof(int));
    if(gradId != -1) {
        mGradient = MainWindow::getInstance()->getLoadedGradientById(gradId);
    }
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

void BoundingBox::writeBoundingBox(QIODevice *target) {
    target->write(rcConstChar(&mType), sizeof(BoundingBoxType));
    gWrite(target, prp_mName);
    target->write(rcConstChar(&mLoadId), sizeof(int));
    target->write(rcConstChar(&mPivotAutoAdjust), sizeof(bool));
    target->write(rcConstChar(&mVisible), sizeof(bool));
    target->write(rcConstChar(&mLocked), sizeof(bool));
    target->write(rcConstChar(&mBlendModeSk), sizeof(SkBlendMode));
    bool hasDurRect = mDurationRectangle != nullptr;
    target->write(rcConstChar(&hasDurRect), sizeof(bool));

    if(hasDurRect) {
        mDurationRectangle->writeDurationRectangle(target);
    }

    mTransformAnimator->writeProperty(target);
    mEffectsAnimators->writeProperty(target);
}

void BoundingBox::readBoundingBox(QIODevice *target) {
    mPivotAutoAdjust = false; // pivot will be read anyway, so temporarly disable adjusting
    gRead(target, prp_mName);
    target->read(rcChar(&mLoadId), sizeof(int));
    bool pivotAutoAdjust;
    target->read(rcChar(&pivotAutoAdjust), sizeof(bool));
    target->read(rcChar(&mVisible), sizeof(bool));
    target->read(rcChar(&mLocked), sizeof(bool));
    target->read(rcChar(&mBlendModeSk), sizeof(SkBlendMode));
    bool hasDurRect;
    target->read(rcChar(&hasDurRect), sizeof(bool));

    if(hasDurRect) {
        if(!mDurationRectangle) createDurationRectangle();
        mDurationRectangle->readDurationRectangle(target);
        updateAfterDurationRectangleShifted();
    }

    mTransformAnimator->readProperty(target);
    mEffectsAnimators->readProperty(target);

    if(hasDurRect) {
        anim_shiftAllKeys(prp_getFrameShift());
    }
    BoundingBox::addLoadedBox(this);
    mPivotAutoAdjust = pivotAutoAdjust;
}

void BoundingBox::writeBoundingBoxDataForLink(QIODevice *target) const {
    target->write(rcConstChar(&mType), sizeof(BoundingBoxType));
    gWrite(target, prp_mName);
    target->write(rcConstChar(&mLoadId), sizeof(int));
    target->write(rcConstChar(&mPivotAutoAdjust), sizeof(bool));
    target->write(rcConstChar(&mVisible), sizeof(bool));
    target->write(rcConstChar(&mLocked), sizeof(bool));
    target->write(rcConstChar(&mBlendModeSk), sizeof(SkBlendMode));
//    bool hasDurRect = mDurationRectangle != nullptr;
//    target->write(rcConstChar(&hasDurRect), sizeof(bool));

//    if(hasDurRect) {
//        mDurationRectangle->writeDurationRectangle(target);
//    }

    mTransformAnimator->writeProperty(target);
    mEffectsAnimators->writeProperty(target);
}

void BoundingBox::readBoundingBoxDataForLink(QIODevice *target) {
    gRead(target, prp_mName);
    target->read(rcChar(&mLoadId), sizeof(int));
    target->read(rcChar(&mPivotAutoAdjust), sizeof(bool));
    target->read(rcChar(&mVisible), sizeof(bool));
    target->read(rcChar(&mLocked), sizeof(bool));
    target->read(rcChar(&mBlendModeSk), sizeof(SkBlendMode));
//    bool hasDurRect;
//    target->read(rcChar(&hasDurRect), sizeof(bool));

//    if(hasDurRect) {
//        if(!mDurationRectangle) createDurationRectangle();
//        mDurationRectangle->readDurationRectangle(target);
//    }

    mTransformAnimator->readProperty(target);
    mEffectsAnimators->readProperty(target);
    BoundingBox::addLoadedBox(this);
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
    mRandomize->writeProperty(target);
    mRandomizeStep->writeProperty(target);
    mSmoothTransform->writeProperty(target);
    mEasing->writeProperty(target);
    mSeed->writeProperty(target);
    mRepeat->writeProperty(target);
}

void DisplacePathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mSegLength->readProperty(target);
    mMaxDev->readProperty(target);
    mSmoothness->readProperty(target);
    mRandomize->readProperty(target);
    mRandomizeStep->readProperty(target);
    mSmoothTransform->readProperty(target);
    mEasing->readProperty(target);
    mSeed->readProperty(target);
    mRepeat->readProperty(target);
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
    BoundingBox *targetBox = mTarget_d.data();
    int targetId;
    if(targetBox) {
        targetId = targetBox->getLoadId();
        if(targetId < 0) {
            targetId = BoundingBox::getLoadedBoxesCount();
            targetBox->setBoxLoadId(targetId);
            BoundingBox::addLoadedBox(targetBox);
        }
    } else {
        targetId = -1;
    }
    target->write(rcConstChar(&targetId), sizeof(int));
}

void BoxTargetProperty::readProperty(QIODevice *target) {
    int targetId;
    target->read(rcChar(&targetId), sizeof(int));
    auto targetBox = BoundingBox::getLoadedBoxById(targetId);
    if(!targetBox && targetId >= 0) {
        BoundingBox::addFunctionWaitingForBoxLoad(
                    SPtrCreate(BoxTargetPropertyWaitingForBoxLoad)(targetId, this) );
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
    } else if(typeT == SUM_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(OperationPathEffect)(
                    GetAsPtr(mParentBox, PathBox), mIsOutline);
    } else if(typeT == GROUP_SUM_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(GroupLastPathSumPathEffect)(
                    GetAsPtr(mParentBox, BoxesGroup), mIsOutline);
    } else if(typeT == LENGTH_PATH_EFFECT) {
        pathEffect = SPtrCreate(LengthPathEffect)(mIsOutline);
    } else if(typeT == SOLIDIFY_PATH_EFFECT) {
        pathEffect = SPtrCreate(SolidifyPathEffect)(mIsOutline);
    } else {
        const QString errMsg = "Invalid path effect type '" +
                QString::number(typeT) + "'.";
        RuntimeThrow(errMsg.toStdString());
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

void PathBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    mPathEffectsAnimators->writeProperty(target);
    mFillPathEffectsAnimators->writeProperty(target);
    mOutlinePathEffectsAnimators->writeProperty(target);
    mFillGradientPoints->writeProperty(target);
    mStrokeGradientPoints->writeProperty(target);
    mFillSettings->writeProperty(target);
    mStrokeSettings->writeProperty(target);
}

void PathBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    mPathEffectsAnimators->readProperty(target);
    mFillPathEffectsAnimators->readProperty(target);
    mOutlinePathEffectsAnimators->readProperty(target);
    mFillGradientPoints->readProperty(target);
    mStrokeGradientPoints->readProperty(target);
    mFillSettings->readProperty(target);
    mStrokeSettings->readProperty(target);
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

void ParticleBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    mTopLeftAnimator->writeProperty(target);
    mBottomRightAnimator->writeProperty(target);
    int nEmitters = mEmitters.count();
    target->write(rcConstChar(&nEmitters), sizeof(int));
    for(const auto& emitter : mEmitters) {
        emitter->writeProperty(target);
    }
}

void ParticleBox::readBoundingBox(QIODevice *target) {
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

void ImageBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    gWrite(target, mImageFilePath);
}

void ImageBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    gRead(target, path);
    setFilePath(path);
}

void Circle::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mHorizontalRadiusAnimator->writeProperty(target);
    mVerticalRadiusAnimator->writeProperty(target);
}

void Circle::readBoundingBox(QIODevice *target) {
    PathBox::readBoundingBox(target);
    mHorizontalRadiusAnimator->readProperty(target);
    mVerticalRadiusAnimator->readProperty(target);
}

void Rectangle::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mRadiusAnimator->writeProperty(target);
    mTopLeftAnimator->writeProperty(target);
    mBottomRightAnimator->writeProperty(target);
}

void Rectangle::readBoundingBox(QIODevice *target) {
    PathBox::readBoundingBox(target);
    mRadiusAnimator->readProperty(target);
    mTopLeftAnimator->readProperty(target);
    mBottomRightAnimator->readProperty(target);
}

void VideoBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    gWrite(target, mSrcFilePath);
}

void VideoBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    gRead(target, path);
    setFilePath(path);
}

bool Tile::writeTile(QIODevice *target) {
    return target->write(rcConstChar(mData),
                         TILE_DIM*TILE_DIM*4*sizeof(uchar)) > 0;
}

void Tile::readTile(QIODevice *target) {
    target->read(rcChar(mData), TILE_DIM*TILE_DIM*4*sizeof(uchar));
    copyDataToDrawer();
}

bool TilesData::writeTilesData(QIODevice *target) {
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            bool writeSuccess = mTiles[j][i]->writeTile(target);
            if(!writeSuccess) return false;
        }
    }
    return true;
}

void TilesData::writeTilesDataFromMemoryOrTmp(QIODevice *target) {
    target->write(rcConstChar(&mWidth), sizeof(ushort));
    target->write(rcConstChar(&mHeight), sizeof(ushort));
    bool noDataInMemory = !mDataStoredInTmpFile && mNoDataInMemory;
    target->write(rcConstChar(&noDataInMemory), sizeof(bool));
    if(mNoDataInMemory) {
        if(mDataStoredInTmpFile) {
            target->write(mTmpFile->readAll());
        }
    } else {
        writeTilesData(target);
    }
}

void TilesData::readTilesData(QIODevice *target) {
    ushort width;
    ushort height;
    target->read(rcChar(&width), sizeof(ushort));
    target->read(rcChar(&height), sizeof(ushort));
    target->read(rcChar(&mNoDataInMemory), sizeof(bool));
    setSize(width, height);
    if(mNoDataInMemory) return;
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->readTile(target);
        }
    }
}

void Surface::writeSurface(QIODevice *target) {
    target->write(rcConstChar(&mWidth), sizeof(int));
    target->write(rcConstChar(&mHeight), sizeof(int));
    mCurrentTiles->writeTilesDataFromMemoryOrTmp(target);
}

void Surface::readSurface(QIODevice *target) {
    target->read(rcChar(&mWidth), sizeof(int));
    target->read(rcChar(&mHeight), sizeof(int));
    mCurrentTiles->readTilesData(target);
}

void SurfaceKey::writeKey(QIODevice *target) {
    Key::writeKey(target);
    mTiles->writeTilesDataFromMemoryOrTmp(target);
}

void SurfaceKey::readKey(QIODevice *target) {
    Key::readKey(target);
    mTiles->readTilesData(target);
}

void AnimatedSurface::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mIsDraft), sizeof(bool));
    target->write(rcConstChar(&mWidth), sizeof(ushort));
    target->write(rcConstChar(&mHeight), sizeof(ushort));
    int nKeys = anim_mKeys.count();
    target->write(rcConstChar(&nKeys), sizeof(int));
    if(nKeys == 0) {
        mCurrentTiles->writeTilesDataFromMemoryOrTmp(target);
    } else {
        for(const auto &key : anim_mKeys) {
            key->writeKey(target);
        }
    }
}

stdsptr<Key> AnimatedSurface::readKey(QIODevice *target) {
    auto newKey = SPtrCreate(SurfaceKey)(this);
    newKey->setTiles(SPtrCreate(TilesData)(0, 0, true));
    newKey->readKey(target);
    return std::move(newKey);
}

void AnimatedSurface::readProperty(QIODevice *target) {
    target->read(rcChar(&mIsDraft), sizeof(bool));
    target->read(rcChar(&mWidth), sizeof(ushort));
    target->read(rcChar(&mHeight), sizeof(ushort));
    int nKeys;
    target->read(rcChar(&nKeys), sizeof(int));
    if(nKeys == 0) {
        mCurrentTiles->readTilesData(target);
    } else {
        for(int i = 0; i < nKeys; i++) {
            anim_appendKey(readKey(target));
        }
    }
    setSize(mParentBox->getWidth(), mParentBox->getHeight());
}

void PaintBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    mTopLeftAnimator->writeProperty(target);
    mBottomRightAnimator->writeProperty(target);
    mMainHandler->writeProperty(target);
}

void PaintBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    mTopLeftAnimator->readProperty(target);
    mBottomRightAnimator->readProperty(target);
    finishSizeSetup();
    mMainHandler->readProperty(target);
}

void ImageSequenceBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    int nFrames = mListOfFrames.count();
    target->write(rcConstChar(&nFrames), sizeof(int));
    for(const QString &frame : mListOfFrames) {
        gWrite(target, frame);
    }
}

void ImageSequenceBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    int nFrames;
    target->read(rcChar(&nFrames), sizeof(int));
    for(int i = 0; i < nFrames; i++) {
        QString frame;
        gRead(target, frame);
        mListOfFrames << frame;
    }
}

void TextBox::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mText->writeProperty(target);
    target->write(rcConstChar(&mAlignment), sizeof(Qt::Alignment));
    qreal fontSize = mFont.pointSizeF();
    QString fontFamily = mFont.family();
    QString fontStyle = mFont.styleName();
    target->write(rcConstChar(&fontSize), sizeof(qreal));
    gWrite(target, fontFamily);
    gWrite(target, fontStyle);
}

void TextBox::readBoundingBox(QIODevice *target) {
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

void BoxesGroup::writeBoundingBox(QIODevice *target) {
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

void BoxesGroup::readChildBoxes(QIODevice *target) {
    int nChildBoxes;
    target->read(rcChar(&nChildBoxes), sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        qsptr<BoundingBox> box;
        BoundingBoxType boxType;
        target->read(rcChar(&boxType), sizeof(BoundingBoxType));
        if(boxType == TYPE_VECTOR_PATH) {
            box = SPtrCreate(VectorPath)();
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
        } else if(boxType == TYPE_GROUP) {
            box = SPtrCreate(BoxesGroup)();
        } else if(boxType == TYPE_PAINT) {
            box = SPtrCreate(PaintBox)();
        } else if(boxType == TYPE_IMAGESQUENCE) {
            box = SPtrCreate(ImageSequenceBox)();
        } else if(boxType == TYPE_INTERNAL_LINK) {
            box = SPtrCreate(InternalLinkBox)(nullptr);
        } else if(boxType == TYPE_EXTERNAL_LINK) {
            box = SPtrCreate(ExternalLinkBox)();
        } else if(boxType == TYPE_INTERNAL_LINK_CANVAS) {
            box = SPtrCreate(InternalLinkCanvas)(nullptr);
        } else {
            QString errMsg = "Invalid box type '" +
                    QString::number(boxType) + "'.";
            RuntimeThrow(errMsg.toStdString());
        }

        box->readBoundingBox(target);
        addContainedBox(box);
    }
}

void BoxesGroup::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    mPathEffectsAnimators->readProperty(target);
    mFillPathEffectsAnimators->readProperty(target);
    mOutlinePathEffectsAnimators->readProperty(target);
    readChildBoxes(target);
}

void PathAnimator::writeProperty(QIODevice * const target) const {
    int nPaths = mSinglePaths.count();
    target->write(rcConstChar(&nPaths), sizeof(int));
    for(const auto& pathAnimator : mSinglePaths) {
        pathAnimator->writeProperty(target);
    }
}

void PathAnimator::readVectorPathAnimator(QIODevice *target) {
    auto pathAnimator = SPtrCreate(VectorPathAnimator)(this);
    pathAnimator->readProperty(target);
    addSinglePathAnimator(pathAnimator);
}

QMatrix PathAnimator::getCombinedTransform() {
    if(mParentBox) mParentBox->getCombinedTransform();
    return QMatrix();
}

void PathAnimator::readProperty(QIODevice *target) {
    int nPaths;
    target->read(rcChar(&nPaths), sizeof(int));
    for(int i = 0; i < nPaths; i++) {
        readVectorPathAnimator(target);
    }
}

void VectorPath::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mPathAnimator->writeProperty(target);
}

void VectorPath::readBoundingBox(QIODevice *target) {
    PathBox::readBoundingBox(target);
    mPathAnimator->readProperty(target);
}

void Canvas::writeBoundingBox(QIODevice *target) {
    BoxesGroup::writeBoundingBox(target);
    target->write(rcConstChar(&mClipToCanvasSize), sizeof(bool));
    target->write(rcConstChar(&mWidth), sizeof(int));
    target->write(rcConstChar(&mHeight), sizeof(int));
    target->write(rcConstChar(&mFps), sizeof(qreal));
    mBackgroundColor->writeProperty(target);
    target->write(rcConstChar(&mMaxFrame), sizeof(int));
    target->write(rcConstChar(&mCanvasTransformMatrix),
                  sizeof(QMatrix));
}

void Canvas::readBoundingBox(QIODevice *target) {
    target->read(rcChar(&mType), sizeof(BoundingBoxType));
    BoxesGroup::readBoundingBox(target);
    target->read(rcChar(&mClipToCanvasSize), sizeof(bool));
    target->read(rcChar(&mWidth), sizeof(int));
    target->read(rcChar(&mHeight), sizeof(int));
    target->read(rcChar(&mFps), sizeof(qreal));
    mBackgroundColor->readProperty(target);
    target->read(rcChar(&mMaxFrame), sizeof(int));
    target->read(rcChar(&mCanvasTransformMatrix), sizeof(QMatrix));
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
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

void Brush::writeBrush(QIODevice *write) {
    gWrite(write, brush_name);
    gWrite(write, collection_name);

    for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSettingInfo setting_info = brush_settings_info[i];
        qreal value = getSettingVal(setting_info.setting);
        write->write(rcConstChar(&value), sizeof(qreal));
    }
}

void Brush::readBrush(QIODevice *read) {
    gRead(read, brush_name);
    gRead(read, collection_name);

    for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        qreal value;
        read->read(rcChar(&value), sizeof(qreal));
        BrushSettingInfo setting_info = brush_settings_info[i];
        setSetting(setting_info.setting, value);
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
    int nCanvases = mCanvasList.count();
    target->write(rcConstChar(&nCanvases), sizeof(int));
    int currentCanvasId = -1;
    int boxLoadId = 0;
    for(const auto &canvas : mCanvasList) {
        boxLoadId = canvas->setBoxLoadId(boxLoadId);
        canvas->writeBoundingBox(target);
        if(canvas.get() == mCurrentCanvas) {
            currentCanvasId = mCurrentCanvas->getLoadId();
        }
    }
    target->write(rcConstChar(&currentCanvasId), sizeof(int));
    for(const auto &canvas : mCanvasList) {
        canvas->clearBoxLoadId();
    }
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
    auto currentCanvas = BoundingBox::getLoadedBoxById(currentCanvasId);
    setCurrentCanvas(GetAsPtr(currentCanvas, Canvas));
}

void MainWindow::loadAVFile(const QString &path) {
    QFile target(path);
    if(target.open(QIODevice::ReadOnly)) {
        FileFooter footer;
        footer.read(&target);
        if(footer.combatybilityMode() ==
                FileFooter::CompatybilityMode::Compatible) {
            QColor qcolT;
            target.read(rcChar(&qcolT), sizeof(QColor));
//            mBrushSettingsWidget->setCurrentQColor(qcolT);
            Brush::setHSV(qcolT.hueF(),
                          qcolT.saturationF(),
                          qcolT.valueF());
//            mBrushSettingsWidget->readBrushesForProject(&target);
            auto gradientWidget = mFillStrokeSettings->getGradientWidget();
            gradientWidget->readGradients(&target);
            mCanvasWindow->readCanvases(&target);

            clearLoadedGradientsList();
            gradientWidget->clearGradientsLoadIds();
            BoundingBox::clearLoadedBoxes();
        } else {
            RuntimeThrow("File incompatible or incomplete " + path.toStdString() + ".");
        }

        target.close();
    } else {
        RuntimeThrow("Could not open file " + path.toStdString() + ".");
    }
}

void MainWindow::saveToFile(const QString &path) {
    QFile target(path);
    if(target.exists()) target.remove();

    if(target.open(QIODevice::WriteOnly)) {
//        QColor color = mBrushSettingsWidget->getCurrentQColor();
//        target.write(rcChar(&color), sizeof(QColor));

//        mBrushSettingsWidget->saveBrushesForProject(&target);
        auto gradientWidget = mFillStrokeSettings->getGradientWidget();
        gradientWidget->setGradientLoadIds();
        gradientWidget->writeGradients(&target);
        mCanvasWindow->writeCanvases(&target);

        clearLoadedGradientsList();
        gradientWidget->clearGradientsLoadIds();

        FileFooter footer;
        footer.write(&target);

        target.close();
    } else {
        RuntimeThrow("Could not open file for writing " +
                     path.toStdString() + ".");
    }

    BoundingBox::clearLoadedBoxes();
}
