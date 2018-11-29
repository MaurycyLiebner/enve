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
#include "Paint/PaintLib/surface.h"
#include "Paint/PaintLib/animatedsurface.h"
#include "Paint/BrushSettings/brushsettingswidget.h"
#include "canvas.h"
#include "durationrectangle.h"
#include "gradientpoints.h"
#include "gradientpoint.h"
#include "qrealkey.h"
#include "GUI/mainwindow.h"
#include "GUI/ColorWidgets/ColorWidgets/gradientwidget.h"
#include <QMessageBox>
#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"

#define FORMAT_STR "AniVect av"
#define CREATOR_VERSION "0.1a"
#define CREATOR_APPLICATION "AniVect"

void writeQString(QIODevice *target,
                 const QString &strToWrite) {
    uint nChars = static_cast<uint>(strToWrite.length());
    target->write(reinterpret_cast<const char*>(&nChars), sizeof(uint));
    target->write(reinterpret_cast<const char*>(strToWrite.utf16()),
                  nChars*sizeof(ushort));
}

void readQString(QIODevice *target,
                 QString& targetStr) {
    uint nChars;
    target->read(reinterpret_cast<char*>(&nChars), sizeof(uint));
    ushort *chars = new ushort[nChars];

    target->read(reinterpret_cast<char*>(chars), nChars*sizeof(ushort));
    targetStr = QString::fromUtf16(chars, static_cast<int>(nChars));

    delete[] chars;
}

struct FileFooter {
    enum CompatybilityMode {
        Compatible,
        NonCompatible
    };

    char formatStr[15] = FORMAT_STR;
    char creatorVersion[15] = CREATOR_VERSION;
    char creatorApplication[15] = CREATOR_APPLICATION;

    CompatybilityMode combatybilityMode() const {
        if(!std::strcmp(formatStr, FORMAT_STR)) {
            return Compatible;
        }
        return NonCompatible;
    }

    void write(QIODevice *target) {
        target->write(reinterpret_cast<char*>(this), sizeof(FileFooter));
    }

    void read(QIODevice *target) {
        target->seek(target->size() - sizeof(FileFooter));
        target->read(reinterpret_cast<char*>(this), sizeof(FileFooter));
        target->seek(0);
    }
};

void BoolProperty::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mValue), sizeof(bool));
}

void BoolProperty::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mValue), sizeof(bool));
}

void BoolPropertyContainer::writeProperty(QIODevice *target) {
    ComplexAnimator::writeProperty(target);
    target->write(reinterpret_cast<char*>(&mValue), sizeof(bool));
}

void BoolPropertyContainer::readProperty(QIODevice *target) {
    ComplexAnimator::readProperty(target);
    bool value;
    target->read(reinterpret_cast<char*>(&value), sizeof(bool));
    setValue(value);
}


void ComboBoxProperty::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mCurrentValue), sizeof(int));
}

void ComboBoxProperty::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mCurrentValue), sizeof(int));
}

void IntProperty::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mMinValue), sizeof(int));
    target->write(reinterpret_cast<char*>(&mMaxValue), sizeof(int));
    target->write(reinterpret_cast<char*>(&mValue), sizeof(int));
}

void IntProperty::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mMinValue), sizeof(int));
    target->read(reinterpret_cast<char*>(&mMaxValue), sizeof(int));
    target->read(reinterpret_cast<char*>(&mValue), sizeof(int));
}

void Key::writeKey(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mRelFrame), sizeof(int));
}

void Key::readKey(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mRelFrame), sizeof(int));
}

void PathContainer::writePathContainer(QIODevice *target) {
    int nPts = mElementsPos.count();
    target->write(reinterpret_cast<char*>(&nPts), sizeof(int)); // number pts
    foreach(const SkPoint &pos, mElementsPos) {
        SkScalar xT, yT;
        xT = pos.x(); yT = pos.y();
        target->write(reinterpret_cast<char*>(&xT), sizeof(SkScalar));
        target->write(reinterpret_cast<char*>(&yT), sizeof(SkScalar));
    }
    target->write(reinterpret_cast<char*>(&mPathClosed), sizeof(bool));
}

void PathContainer::readPathContainer(QIODevice *target) {
    int nPts;
    target->read(reinterpret_cast<char*>(&nPts), sizeof(int));
    for(int i = 0; i < nPts; i++) {
        SkScalar xT, yT;
        target->read(reinterpret_cast<char*>(&xT), sizeof(SkScalar));
        target->read(reinterpret_cast<char*>(&yT), sizeof(SkScalar));
        mElementsPos.append(SkPoint::Make(xT, yT));
    }
    target->read(reinterpret_cast<char*>(&mPathClosed), sizeof(bool));
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

void VectorPathAnimator::writeProperty(QIODevice *target) {
    int nNodes = mNodeSettings.count();
    target->write(reinterpret_cast<char*>(&nNodes), sizeof(int));
    foreach(const NodeSettingsSPtr &nodeSettings, mNodeSettings) {
        nodeSettings->write(target);
    }

    int nKeys = anim_mKeys.count();
    target->write(reinterpret_cast<char*>(&nKeys), sizeof(int));
    foreach(const KeySPtr &key, anim_mKeys) {
        key->writeKey(target);
    }

    writePathContainer(target);
}

KeySPtr VectorPathAnimator::readKey(QIODevice *target) {
    PathKeySPtr newKey = SPtrCreate(PathKey)(this);

    newKey->readKey(target);
    return newKey;
}

void NodeSettings::write(QIODevice* target) {
    target->write(reinterpret_cast<char*>(&ctrlsMode),
                 sizeof(CtrlsMode));
    target->write(reinterpret_cast<char*>(&startEnabled),
                 sizeof(bool));
    target->write(reinterpret_cast<char*>(&endEnabled),
                 sizeof(bool));
}

void NodeSettings::read(QIODevice* target) {
    target->read(reinterpret_cast<char*>(&ctrlsMode),
                 sizeof(CtrlsMode));
    target->read(reinterpret_cast<char*>(&startEnabled),
                 sizeof(bool));
    target->read(reinterpret_cast<char*>(&endEnabled),
                 sizeof(bool));
}

void VectorPathAnimator::readProperty(QIODevice *target) {
    int nNodes;
    target->read(reinterpret_cast<char*>(&nNodes), sizeof(int));
    for(int i = 0; i < nNodes; i++) {
        NodeSettingsSPtr nodeSettings =
                SPtrCreate(NodeSettings)();
        nodeSettings->read(target);
        insertNodeSettingsForNodeId(i, nodeSettings);
    }

    int nKeys;
    target->read(reinterpret_cast<char*>(&nKeys), sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        anim_appendKey(readKey(target),
                       false,
                       false);
    }

    readPathContainer(target);
    updateNodePointsFromElements();
}

void Animator::writeSelectedKeys(QIODevice* target) {
    int nKeys = anim_mSelectedKeys.count();
    target->write(reinterpret_cast<char*>(&nKeys), sizeof(int));
    Q_FOREACH(Key *key, anim_mSelectedKeys) {
        key->writeKey(target);
    }
}

void QrealKey::writeKey(QIODevice *target) {
    Key::writeKey(target);
    target->write(reinterpret_cast<char*>(&mValue), sizeof(qreal));

    target->write(reinterpret_cast<char*>(&mStartEnabled), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mStartFrame), sizeof(qreal));
    target->write(reinterpret_cast<char*>(&mStartValue), sizeof(qreal));

    target->write(reinterpret_cast<char*>(&mEndEnabled), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mEndFrame), sizeof(qreal));
    target->write(reinterpret_cast<char*>(&mEndValue), sizeof(qreal));
}

void QrealKey::readKey(QIODevice *target) {
    Key::readKey(target);
    target->read(reinterpret_cast<char*>(&mValue), sizeof(qreal));

    target->read(reinterpret_cast<char*>(&mStartEnabled), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mStartFrame), sizeof(qreal));
    target->read(reinterpret_cast<char*>(&mStartValue), sizeof(qreal));

    target->read(reinterpret_cast<char*>(&mEndEnabled), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mEndFrame), sizeof(qreal));
    target->read(reinterpret_cast<char*>(&mEndValue), sizeof(qreal));
}

void QrealAnimator::writeProperty(QIODevice *target) {
    int nKeys = anim_mKeys.count();
    target->write(reinterpret_cast<char*>(&nKeys), sizeof(int));
    foreach(const KeySPtr &key, anim_mKeys) {
        key->writeKey(target);
    }

    target->write(reinterpret_cast<char*>(&mCurrentValue), sizeof(qreal));

    bool hasRandomGenerator = !mRandomGenerator.isNull();
    target->write(reinterpret_cast<char*>(&hasRandomGenerator), sizeof(bool));
    if(hasRandomGenerator) {
        mRandomGenerator->writeProperty(target);
    }
}

KeySPtr QrealAnimator::readKey(QIODevice *target) {
    QrealKeySPtr newKey = SPtrCreate(QrealKey)(this);
    newKey->readKey(target);
    return newKey;
}

void RandomQrealGenerator::writeProperty(QIODevice *target) {
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
    target->read(reinterpret_cast<char*>(&nKeys), sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        anim_appendKey(readKey(target), false, false);
    }

    qreal val;
    target->read(reinterpret_cast<char*>(&val), sizeof(qreal));
    qra_setCurrentValue(val);
    bool hasRandomGenerator;
    target->read(reinterpret_cast<char*>(&hasRandomGenerator), sizeof(bool));
    if(hasRandomGenerator) {
        RandomQrealGeneratorQSPtr generator =
                SPtrCreate(RandomQrealGenerator)(0, 9999);
        generator->readProperty(target);
        setGenerator(generator);
    }
}

void QPointFAnimator::writeProperty(QIODevice *target) {
    mXAnimator->writeProperty(target);
    mYAnimator->writeProperty(target);
}

void QPointFAnimator::readProperty(QIODevice *target) {
    mXAnimator->readProperty(target);
    mYAnimator->readProperty(target);
}

void ColorAnimator::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mColorMode), sizeof(ColorMode));
    mVal1Animator->writeProperty(target);
    mVal2Animator->writeProperty(target);
    mVal3Animator->writeProperty(target);
    mAlphaAnimator->writeProperty(target);
}

void ColorAnimator::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mColorMode), sizeof(ColorMode));
    setColorMode(mColorMode);
    mVal1Animator->readProperty(target);
    mVal2Animator->readProperty(target);
    mVal3Animator->readProperty(target);
    mAlphaAnimator->readProperty(target);
}

void QStringKey::writeKey(QIODevice *target) {
    Key::writeKey(target);
    writeQString(target, mText);
}

void QStringKey::readKey(QIODevice *target) {
    Key::readKey(target);
    readQString(target, mText);
}

void QStringAnimator::writeProperty(QIODevice *target) {
    int nKeys = anim_mKeys.count();
    target->write(reinterpret_cast<char*>(&nKeys), sizeof(int));
    foreach(const KeySPtr &key, anim_mKeys) {
        key->writeKey(target);
    }
    writeQString(target, mCurrentText);
}

void QStringAnimator::readProperty(QIODevice *target) {
    int nKeys;
    target->read(reinterpret_cast<char*>(&nKeys), sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        QStringKeySPtr newKey = SPtrCreate(QStringKey)("", 0, this);
        newKey->readKey(target);
        anim_appendKey(newKey, false, false);
    }
    readQString(target, mCurrentText);
}

void PixmapEffect::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mType), sizeof(PixmapEffectType));
    target->write(reinterpret_cast<char*>(&mVisible), sizeof(bool));
}

void PixmapEffect::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mVisible), sizeof(bool));
}

void BlurEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mHighQuality->readProperty(target);
    mBlurRadius->readProperty(target);
}

void BlurEffect::writeProperty(QIODevice *target) {
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

void ShadowEffect::writeProperty(QIODevice *target) {
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

void DesaturateEffect::writeProperty(QIODevice *target) {
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

void ColorizeEffect::writeProperty(QIODevice *target) {
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

void ReplaceColorEffect::writeProperty(QIODevice *target) {
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

void ContrastEffect::writeProperty(QIODevice *target) {
    PixmapEffect::writeProperty(target);
    mContrastAnimator->writeProperty(target);
}

void BrightnessEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mBrightnessAnimator->readProperty(target);
}

void BrightnessEffect::writeProperty(QIODevice *target) {
    PixmapEffect::writeProperty(target);
    mBrightnessAnimator->writeProperty(target);
}

void SampledMotionBlurEffect::readProperty(QIODevice *target) {
    PixmapEffect::readProperty(target);
    mOpacity->readProperty(target);
    mNumberSamples->readProperty(target);
    mFrameStep->readProperty(target);
}

void SampledMotionBlurEffect::writeProperty(QIODevice *target) {
    PixmapEffect::writeProperty(target);
    mOpacity->writeProperty(target);
    mNumberSamples->writeProperty(target);
    mFrameStep->writeProperty(target);
}

void EffectAnimators::writeProperty(QIODevice *target) {
    int nEffects = ca_mChildAnimators.count();
    target->write(reinterpret_cast<char*>(&nEffects), sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        effect->writeProperty(target);
    }
}

void EffectAnimators::readPixmapEffect(QIODevice *target) {
    PixmapEffectType typeT;
    target->read(reinterpret_cast<char*>(&typeT), sizeof(PixmapEffectType));
    PixmapEffectQSPtr effect;
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
        return;
    }
    effect->readProperty(target);
    addEffect(effect);
}

void EffectAnimators::readProperty(QIODevice *target) {
    int nEffects;
    target->read(reinterpret_cast<char*>(&nEffects), sizeof(int));
    for(int i = 0; i < nEffects; i++) {
        readPixmapEffect(target);
    }
}

void BasicTransformAnimator::writeProperty(QIODevice *target) {
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

void BoxTransformAnimator::writeProperty(QIODevice *target) {
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

void GradientPoints::writeProperty(QIODevice *target) {
    mStartAnimator->writeProperty(target);
    mEndAnimator->writeProperty(target);
}

void GradientPoints::readProperty(QIODevice *target) {
    mStartAnimator->readProperty(target);
    mEndAnimator->readProperty(target);
}

void Gradient::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mLoadId), sizeof(int));
    int nColors = mColors.count();
    target->write(reinterpret_cast<char*>(&nColors), sizeof(int));
    foreach(const ColorAnimatorQSPtr& color, mColors) {
        color->writeProperty(target);
    }
}

void Gradient::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mLoadId), sizeof(int));
    int nColors;
    target->read(reinterpret_cast<char*>(&nColors), sizeof(int));
    for(int i = 0; i < nColors; i++) {
        ColorAnimatorQSPtr colorAnim = SPtrCreate(ColorAnimator)();
        colorAnim->readProperty(target);
        addColorToList(colorAnim, false);
    }
}

void StrokeSettings::writeProperty(QIODevice *target) {
    PaintSettings::writeProperty(target);
    mLineWidth->writeProperty(target);
    target->write(reinterpret_cast<char*>(&mCapStyle), sizeof(Qt::PenCapStyle));
    target->write(reinterpret_cast<char*>(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    target->write(reinterpret_cast<char*>(&mOutlineCompositionMode),
               sizeof(QPainter::CompositionMode));
}

void StrokeSettings::readProperty(QIODevice *target) {
    PaintSettings::readProperty(target);
    mLineWidth->readProperty(target);
    target->read(reinterpret_cast<char*>(&mCapStyle), sizeof(Qt::PenCapStyle));
    target->read(reinterpret_cast<char*>(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    target->read(reinterpret_cast<char*>(&mOutlineCompositionMode),
                sizeof(QPainter::CompositionMode));
}

void PaintSettings::writeProperty(QIODevice *target) {
    mGradientPoints->writeProperty(target);
    mColor->writeProperty(target);
    target->write(reinterpret_cast<char*>(&mPaintType), sizeof(PaintType));
    int gradId;
    if(mGradient == nullptr) {
        gradId = -1;
    } else {
        gradId = mGradient->getLoadId();
    }
    target->write(reinterpret_cast<char*>(&mGradientLinear), sizeof(bool));
    target->write(reinterpret_cast<char*>(&gradId), sizeof(int));
}

void PaintSettings::readProperty(QIODevice *target) {
    mGradientPoints->readProperty(target);
    mColor->readProperty(target);
    target->read(reinterpret_cast<char*>(&mPaintType), sizeof(PaintType));
    int gradId;
    target->read(reinterpret_cast<char*>(&mGradientLinear), sizeof(bool));
    target->read(reinterpret_cast<char*>(&gradId), sizeof(int));
    if(gradId != -1) {
        mGradient = MainWindow::getInstance()->getLoadedGradientById(gradId);
    }
}

void DurationRectangle::writeDurationRectangle(QIODevice *target) {
    int minFrame = getMinFrame();
    int maxFrame = getMaxFrame();
    int framePos = getFramePos();
    target->write(reinterpret_cast<char*>(&minFrame), sizeof(int));
    target->write(reinterpret_cast<char*>(&maxFrame), sizeof(int));
    target->write(reinterpret_cast<char*>(&framePos), sizeof(int));
}

void DurationRectangle::readDurationRectangle(QIODevice *target) {
    int minFrame;
    int maxFrame;
    int framePos;
    target->read(reinterpret_cast<char*>(&minFrame), sizeof(int));
    target->read(reinterpret_cast<char*>(&maxFrame), sizeof(int));
    target->read(reinterpret_cast<char*>(&framePos), sizeof(int));
    setMinFrame(minFrame);
    setMaxFrame(maxFrame);
    setFramePos(framePos);
}

void FixedLenAnimationRect::writeDurationRectangle(QIODevice *target) {
    DurationRectangle::writeDurationRectangle(target);
    target->write(reinterpret_cast<char*>(&mBoundToAnimation), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mMinAnimationFrame), sizeof(int));
    target->write(reinterpret_cast<char*>(&mMaxAnimationFrame), sizeof(int));
}

void FixedLenAnimationRect::readDurationRectangle(QIODevice *target) {
    DurationRectangle::readDurationRectangle(target);
    int minFrame;
    int maxFrame;
    target->read(reinterpret_cast<char*>(&mBoundToAnimation), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->read(reinterpret_cast<char*>(&minFrame), sizeof(int));
    target->read(reinterpret_cast<char*>(&maxFrame), sizeof(int));
    setMinAnimationFrame(minFrame);
    setMaxAnimationFrame(maxFrame);
}

void BoundingBox::writeBoundingBox(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mType), sizeof(BoundingBoxType));
    writeQString(target, prp_mName);
    target->write(reinterpret_cast<char*>(&mLoadId), sizeof(int));
    target->write(reinterpret_cast<char*>(&mPivotAutoAdjust), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mVisible), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mLocked), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mBlendModeSk), sizeof(SkBlendMode));
    bool hasDurRect = mDurationRectangle != nullptr;
    target->write(reinterpret_cast<char*>(&hasDurRect), sizeof(bool));

    if(hasDurRect) {
        mDurationRectangle->writeDurationRectangle(target);
    }

    mTransformAnimator->writeProperty(target);
    mEffectsAnimators->writeProperty(target);
}

void BoundingBox::readBoundingBox(QIODevice *target) {
    mPivotAutoAdjust = false; // pivot will be read anyway, so temporarly disable adjusting
    readQString(target, prp_mName);
    target->read(reinterpret_cast<char*>(&mLoadId), sizeof(int));
    bool pivotAutoAdjust;
    target->read(reinterpret_cast<char*>(&pivotAutoAdjust), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mVisible), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mLocked), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mBlendModeSk), sizeof(SkBlendMode));
    bool hasDurRect;
    target->read(reinterpret_cast<char*>(&hasDurRect), sizeof(bool));

    if(hasDurRect) {
        if(mDurationRectangle == nullptr) createDurationRectangle();
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

void BoundingBox::writeBoundingBoxDataForLink(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mType), sizeof(BoundingBoxType));
    writeQString(target, prp_mName);
    target->write(reinterpret_cast<char*>(&mLoadId), sizeof(int));
    target->write(reinterpret_cast<char*>(&mPivotAutoAdjust), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mVisible), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mLocked), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mBlendModeSk), sizeof(SkBlendMode));
//    bool hasDurRect = mDurationRectangle != nullptr;
//    target->write(reinterpret_cast<char*>(&hasDurRect), sizeof(bool));

//    if(hasDurRect) {
//        mDurationRectangle->writeDurationRectangle(target);
//    }

    mTransformAnimator->writeProperty(target);
    mEffectsAnimators->writeProperty(target);
}

void BoundingBox::readBoundingBoxDataForLink(QIODevice *target) {
    readQString(target, prp_mName);
    target->read(reinterpret_cast<char*>(&mLoadId), sizeof(int));
    target->read(reinterpret_cast<char*>(&mPivotAutoAdjust), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mVisible), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mLocked), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mBlendModeSk), sizeof(SkBlendMode));
//    bool hasDurRect;
//    target->read(reinterpret_cast<char*>(&hasDurRect), sizeof(bool));

//    if(hasDurRect) {
//        if(mDurationRectangle == nullptr) createDurationRectangle();
//        mDurationRectangle->readDurationRectangle(target);
//    }

    mTransformAnimator->readProperty(target);
    mEffectsAnimators->readProperty(target);
    BoundingBox::addLoadedBox(this);
}

void PathEffect::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mPathEffectType), sizeof(PathEffectType));
    target->write(reinterpret_cast<char*>(&mVisible), sizeof(bool));
}

void PathEffect::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mVisible), sizeof(bool));
}

void DisplacePathEffect::writeProperty(QIODevice *target) {
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

void DuplicatePathEffect::writeProperty(QIODevice *target) {
    PathEffect::writeProperty(target);
    mTranslation->writeProperty(target);
}

void DuplicatePathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mTranslation->readProperty(target);
}

void LengthPathEffect::writeProperty(QIODevice *target) {
    PathEffect::writeProperty(target);
    mLength->writeProperty(target);
    mReverse->writeProperty(target);
}

void LengthPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mLength->readProperty(target);
    mReverse->readProperty(target);
}

void SolidifyPathEffect::writeProperty(QIODevice *target) {
    PathEffect::writeProperty(target);
    mDisplacement->writeProperty(target);
}

void SolidifyPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mDisplacement->readProperty(target);
}

void BoxTargetProperty::writeProperty(QIODevice *target) {
    BoundingBox *targetBox = mTarget_d.data();
    int targetId;
    if(targetBox == nullptr) {
        targetId = -1;
    } else {
        targetId = targetBox->getLoadId();
        if(targetId < 0) {
            targetId = BoundingBox::getLoadedBoxesCount();
            targetBox->setBoxLoadId(targetId);
            BoundingBox::addLoadedBox(targetBox);
        }
    }
    target->write(reinterpret_cast<char*>(&targetId), sizeof(int));
}

void BoxTargetProperty::readProperty(QIODevice *target) {
    int targetId;
    target->read(reinterpret_cast<char*>(&targetId), sizeof(int));
    BoundingBox* targetBox = BoundingBox::getLoadedBoxById(targetId);
    if(targetBox == nullptr && targetId >= 0) {
        BoundingBox::addFunctionWaitingForBoxLoad(
                    SPtrCreate(BoxTargetPropertyWaitingForBoxLoad)(targetId, this) );
    } else {
        setTarget(targetBox);
    }
}

void OperationPathEffect::writeProperty(QIODevice *target) {
    PathEffect::writeProperty(target);
    mBoxTarget->writeProperty(target);
}

void OperationPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mBoxTarget->readProperty(target);
}

void PathEffectAnimators::writeProperty(QIODevice *target) {   
    int nEffects = ca_mChildAnimators.count();
    target->write(reinterpret_cast<char*>(&nEffects), sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        effect->writeProperty(target);
    }
}

void PathEffectAnimators::readPathEffect(QIODevice *target) {
    PathEffectType typeT;
    target->read(reinterpret_cast<char*>(&typeT), sizeof(PathEffectType));
    PathEffectQSPtr pathEffect;
    if(typeT == DISPLACE_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(DisplacePathEffect)(mIsOutline);
    } else if(typeT == DUPLICATE_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(DuplicatePathEffect)(mIsOutline);
    } else if(typeT == SUM_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(OperationPathEffect)(getAsPtr(mParentBox, PathBox), mIsOutline);
    } else if(typeT == GROUP_SUM_PATH_EFFECT) {
        pathEffect =
                SPtrCreate(GroupLastPathSumPathEffect)(
                    getAsPtr(mParentBox, BoxesGroup),
                    mIsOutline);
    } else if(typeT == LENGTH_PATH_EFFECT) {
        pathEffect = SPtrCreate(LengthPathEffect)(mIsOutline);
    } else {
        return;
    }
    pathEffect->readProperty(target);
    addEffect(pathEffect);
}

void PathEffectAnimators::readProperty(QIODevice *target) {
    int nEffects;
    target->read(reinterpret_cast<char*>(&nEffects), sizeof(int));
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

void ParticleEmitter::writeProperty(QIODevice *target) {
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
    target->write(reinterpret_cast<char*>(&nEmitters), sizeof(int));
    foreach(const ParticleEmitterQSPtr& emitter, mEmitters) {
        emitter->writeProperty(target);
    }
}

void ParticleBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    mTopLeftAnimator->readProperty(target);
    mBottomRightAnimator->readProperty(target);
    int nEmitters;
    target->read(reinterpret_cast<char*>(&nEmitters), sizeof(int));
    for(int i = 0; i < nEmitters; i++) {
        ParticleEmitterQSPtr emitter = SPtrCreate(ParticleEmitter)(this);
        emitter->readProperty(target);
        addEmitter(emitter);
    }
}

void ImageBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    writeQString(target, mImageFilePath);
}

void ImageBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    readQString(target, path);
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
    writeQString(target, mSrcFilePath);
}

void VideoBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    readQString(target, path);
    setFilePath(path);
}

bool Tile::writeTile(QIODevice *target) {
    return target->write(reinterpret_cast<char*>(mData), TILE_DIM*TILE_DIM*4*sizeof(uchar)) > 0;
}

void Tile::readTile(QIODevice *target) {
    target->read(reinterpret_cast<char*>(mData), TILE_DIM*TILE_DIM*4*sizeof(uchar));
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
    target->write(reinterpret_cast<char*>(&mWidth), sizeof(ushort));
    target->write(reinterpret_cast<char*>(&mHeight), sizeof(ushort));
    bool noDataInMemory = !mDataStoredInTmpFile && mNoDataInMemory;
    target->write(reinterpret_cast<char*>(&noDataInMemory), sizeof(bool));
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
    target->read(reinterpret_cast<char*>(&width), sizeof(ushort));
    target->read(reinterpret_cast<char*>(&height), sizeof(ushort));
    target->read(reinterpret_cast<char*>(&mNoDataInMemory), sizeof(bool));
    setSize(width, height);
    if(mNoDataInMemory) return;
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->readTile(target);
        }
    }
}

void Surface::writeSurface(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mWidth), sizeof(int));
    target->write(reinterpret_cast<char*>(&mHeight), sizeof(int));
    mCurrentTiles->writeTilesDataFromMemoryOrTmp(target);
}

void Surface::readSurface(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mWidth), sizeof(int));
    target->read(reinterpret_cast<char*>(&mHeight), sizeof(int));
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

void AnimatedSurface::writeProperty(QIODevice *target) {
    target->write(reinterpret_cast<char*>(&mIsDraft), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mWidth), sizeof(ushort));
    target->write(reinterpret_cast<char*>(&mHeight), sizeof(ushort));
    int nKeys = anim_mKeys.count();
    target->write(reinterpret_cast<char*>(&nKeys), sizeof(int));
    if(nKeys == 0) {
        mCurrentTiles->writeTilesDataFromMemoryOrTmp(target);
    } else {
        foreach(const KeySPtr &key, anim_mKeys) {
            key->writeKey(target);
        }
    }
}

KeySPtr AnimatedSurface::readKey(QIODevice *target) {
    SurfaceKeySPtr newKey = SPtrCreate(SurfaceKey)(this);
    newKey->setTiles(SPtrCreate(TilesData)(0, 0, true));
    newKey->readKey(target);
    return newKey;
}

void AnimatedSurface::readProperty(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mIsDraft), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mWidth), sizeof(ushort));
    target->read(reinterpret_cast<char*>(&mHeight), sizeof(ushort));
    int nKeys;
    target->read(reinterpret_cast<char*>(&nKeys), sizeof(int));
    if(nKeys == 0) {
        mCurrentTiles->readTilesData(target);
    } else {
        for(int i = 0; i < nKeys; i++) {
            anim_appendKey(readKey(target), false, false);
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
    target->write(reinterpret_cast<char*>(&nFrames), sizeof(int));
    foreach(const QString &frame, mListOfFrames) {
        writeQString(target, frame);
    }
}

void ImageSequenceBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    int nFrames;
    target->read(reinterpret_cast<char*>(&nFrames), sizeof(int));
    for(int i = 0; i < nFrames; i++) {
        QString frame;
        readQString(target, frame);
        mListOfFrames << frame;
    }
}

void TextBox::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mText->writeProperty(target);
    target->write(reinterpret_cast<char*>(&mAlignment), sizeof(Qt::Alignment));
    qreal fontSize = mFont.pointSizeF();
    QString fontFamily = mFont.family();
    QString fontStyle = mFont.styleName();
    target->write(reinterpret_cast<char*>(&fontSize), sizeof(qreal));
    writeQString(target, fontFamily);
    writeQString(target, fontStyle);
}

void TextBox::readBoundingBox(QIODevice *target) {
    PathBox::readBoundingBox(target);
    mText->readProperty(target);
    target->read(reinterpret_cast<char*>(&mAlignment), sizeof(Qt::Alignment));
    qreal fontSize;
    QString fontFamily;
    QString fontStyle;
    target->read(reinterpret_cast<char*>(&fontSize), sizeof(qreal));
    readQString(target, fontFamily);
    readQString(target, fontStyle);
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
    target->write(reinterpret_cast<char*>(&nChildBoxes), sizeof(int));
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        child->writeBoundingBox(target);
    }
}

void BoxesGroup::readChildBoxes(QIODevice *target) {
    int nChildBoxes;
    target->read(reinterpret_cast<char*>(&nChildBoxes), sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        BoundingBoxQSPtr box;
        BoundingBoxType boxType;
        target->read(reinterpret_cast<char*>(&boxType), sizeof(BoundingBoxType));
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
            qDebug() << "Corrupted file - invalid box type";
            break;
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

void PathAnimator::writeProperty(QIODevice *target) {
    int nPaths = mSinglePaths.count();
    target->write(reinterpret_cast<char*>(&nPaths), sizeof(int));
    foreach(const VectorPathAnimatorQSPtr& pathAnimator, mSinglePaths) {
        pathAnimator->writeProperty(target);
    }
}

void PathAnimator::readVectorPathAnimator(QIODevice *target) {
    VectorPathAnimatorQSPtr pathAnimator =
            SPtrCreate(VectorPathAnimator)(this);
    pathAnimator->readProperty(target);
    addSinglePathAnimator(pathAnimator, false);
}

QMatrix PathAnimator::getCombinedTransform() {
    if(mParentBox == nullptr) return QMatrix();
    return mParentBox->getCombinedTransform();
}

void PathAnimator::readProperty(QIODevice *target) {
    int nPaths;
    target->read(reinterpret_cast<char*>(&nPaths), sizeof(int));
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
    target->write(reinterpret_cast<char*>(&mClipToCanvasSize), sizeof(bool));
    target->write(reinterpret_cast<char*>(&mWidth), sizeof(int));
    target->write(reinterpret_cast<char*>(&mHeight), sizeof(int));
    target->write(reinterpret_cast<char*>(&mFps), sizeof(qreal));
    mBackgroundColor->writeProperty(target);
    target->write(reinterpret_cast<char*>(&mMaxFrame), sizeof(int));
    target->write(reinterpret_cast<char*>(&mCanvasTransformMatrix), sizeof(QMatrix));
}

void Canvas::readBoundingBox(QIODevice *target) {
    target->read(reinterpret_cast<char*>(&mType), sizeof(BoundingBoxType));
    BoxesGroup::readBoundingBox(target);
    target->read(reinterpret_cast<char*>(&mClipToCanvasSize), sizeof(bool));
    target->read(reinterpret_cast<char*>(&mWidth), sizeof(int));
    target->read(reinterpret_cast<char*>(&mHeight), sizeof(int));
    target->read(reinterpret_cast<char*>(&mFps), sizeof(qreal));
    mBackgroundColor->readProperty(target);
    target->read(reinterpret_cast<char*>(&mMaxFrame), sizeof(int));
    target->read(reinterpret_cast<char*>(&mCanvasTransformMatrix), sizeof(QMatrix));
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
}

void GradientWidget::writeGradients(QIODevice *target) {
    int nGradients = mGradients.count();
    target->write(reinterpret_cast<char*>(&nGradients), sizeof(int));
    foreach(const QSharedPointer<Gradient> &gradient, mGradients) {
        gradient->writeProperty(target);
    }
}

void GradientWidget::readGradients(QIODevice *target) {
    int nGradients;
    target->read(reinterpret_cast<char*>(&nGradients), sizeof(int));
    for(int i = 0; i < nGradients; i++) {
        GradientQSPtr gradient = SPtrCreate(Gradient)();
        gradient->readProperty(target);
        addGradientToList(gradient);
        MainWindow::getInstance()->addLoadedGradient(gradient.get());
    }
}

void Brush::writeBrush(QIODevice *write) {
    writeQString(write, brush_name);
    writeQString(write, collection_name);

    for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSettingInfo setting_info = brush_settings_info[i];
        qreal value = getSettingVal(setting_info.setting);
        write->write(reinterpret_cast<const char*>(&value), sizeof(qreal));
    }
}

void Brush::readBrush(QIODevice *read) {
    readQString(read, brush_name);
    readQString(read, collection_name);

    for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        qreal value;
        read->read(reinterpret_cast<char*>(&value), sizeof(qreal));
        BrushSettingInfo setting_info = brush_settings_info[i];
        setSetting(setting_info.setting, value);
    }
}

QColor readQColor(QIODevice *read) {
    QColor qcolT;
    read->read(reinterpret_cast<char*>(&qcolT), sizeof(QColor));
    return qcolT;
}

void writeQColor(const QColor& qcol, QIODevice *write) {
    write->write(reinterpret_cast<const char*>(&qcol), sizeof(QColor));
}

void CanvasWindow::writeCanvases(QIODevice *target) {
    int nCanvases = mCanvasList.count();
    target->write(reinterpret_cast<char*>(&nCanvases), sizeof(int));
    int currentCanvasId = -1;
    int boxLoadId = 0;
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
        boxLoadId = canvas->setBoxLoadId(boxLoadId);
        canvas->writeBoundingBox(target);
        if(canvas.get() == mCurrentCanvas) {
            currentCanvasId = mCurrentCanvas->getLoadId();
        }
    }
    target->write(reinterpret_cast<char*>(&currentCanvasId), sizeof(int));
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
        canvas->clearBoxLoadId();
    }
}

void CanvasWindow::readCanvases(QIODevice *target) {
    int nCanvases;
    target->read(reinterpret_cast<char*>(&nCanvases), sizeof(int));
    for(int i = 0; i < nCanvases; i++) {
        CanvasQSPtr canvas =
                SPtrCreate(Canvas)(this);
        canvas->readBoundingBox(target);
        MainWindow::getInstance()->addCanvas(canvas);
    }
    int currentCanvasId;
    target->read(reinterpret_cast<char*>(&currentCanvasId), sizeof(int));
    setCurrentCanvas(
                getAsPtr(BoundingBox::getLoadedBoxById(currentCanvasId), Canvas));
}

void MainWindow::loadAVFile(const QString &path) {
    QFile target(path);
    if(target.open(QIODevice::ReadOnly) ) {
        FileFooter footer;
        footer.read(&target);
        if(footer.combatybilityMode() ==
                FileFooter::CompatybilityMode::Compatible) {
            QColor qcolT;
            target.read(reinterpret_cast<char*>(&qcolT), sizeof(QColor));
            mBrushSettingsWidget->setCurrentQColor(qcolT);
            Brush::setHSV(qcolT.hueF(),
                          qcolT.saturationF(),
                          qcolT.valueF());
            mBrushSettingsWidget->readBrushesForProject(&target);
            GradientWidget *gradientWidget =
                    mFillStrokeSettings->getGradientWidget();
            gradientWidget->readGradients(&target);
            mCanvasWindow->readCanvases(&target);

            clearLoadedGradientsList();
            gradientWidget->clearGradientsLoadIds();
            BoundingBox::clearLoadedBoxes();
        } else {
            QMessageBox::critical(this, tr("File Load Fail"),
                                  tr("The target you tried to load is incompatible,\n"
                                     "or damaged."),
                                  QMessageBox::Ok,
                                  QMessageBox::Ok);
        }

        target.close();
    }
}

void MainWindow::saveToFile(const QString &path) {
    disable();
    QFile target(path);
    if(target.exists()) {
        target.remove();
    }

    if(target.open(QIODevice::WriteOnly) ) {
        QColor color = mBrushSettingsWidget->getCurrentQColor();
        target.write(reinterpret_cast<char*>(&color), sizeof(QColor));

        mBrushSettingsWidget->saveBrushesForProject(&target);
        GradientWidget *gradientWidget =
                mFillStrokeSettings->getGradientWidget();
        gradientWidget->setGradientLoadIds();
        gradientWidget->writeGradients(&target);
        mCanvasWindow->writeCanvases(&target);

        clearLoadedGradientsList();
        gradientWidget->clearGradientsLoadIds();

        FileFooter footer;
        footer.write(&target);

        target.close();
    }

    BoundingBox::clearLoadedBoxes();

    enable();
}
