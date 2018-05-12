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
#include "mainwindow.h"
#include "Colors/ColorWidgets/gradientwidget.h"
#include <QMessageBox>

#define FORMAT_STR "AniVect av"
#define CREATOR_VERSION "0.1a"
#define CREATOR_APPLICATION "AniVect"

void writeQString(QIODevice *target,
                 const QString &strToWrite) {
    int nChars = strToWrite.length();
    target->write((char*)&nChars, sizeof(int));
    target->write((char*)strToWrite.utf16(), nChars*sizeof(ushort));
}

void readQString(QIODevice *target,
                QString *targetStr) {
    int nChars;
    target->read((char*)&nChars, sizeof(int));
    ushort *chars = new ushort[nChars];

    target->read((char*)chars, nChars*sizeof(ushort));
    *targetStr = QString::fromUtf16(chars, nChars);
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
        target->write((char*)this, sizeof(FileFooter));
    }

    void read(QIODevice *target) {
        target->seek(target->size() - sizeof(FileFooter));
        target->read((char*)this, sizeof(FileFooter));
        target->seek(0);
    }
};

void BoolProperty::writeProperty(QIODevice *target) {
    target->write((char*)&mValue, sizeof(bool));
}

void BoolProperty::readProperty(QIODevice *target) {
    target->read((char*)&mValue, sizeof(bool));
}

void BoolPropertyContainer::writeProperty(QIODevice *target) {
    ComplexAnimator::writeProperty(target);
    target->write((char*)&mValue, sizeof(bool));
}

void BoolPropertyContainer::readProperty(QIODevice *target) {
    ComplexAnimator::readProperty(target);
    target->read((char*)&mValue, sizeof(bool));
}


void ComboBoxProperty::writeProperty(QIODevice *target) {
    target->write((char*)&mCurrentValue, sizeof(int));
}

void ComboBoxProperty::readProperty(QIODevice *target) {
    target->read((char*)&mCurrentValue, sizeof(int));
}

void IntProperty::writeProperty(QIODevice *target) {
    target->write((char*)&mMinValue, sizeof(int));
    target->write((char*)&mMaxValue, sizeof(int));
    target->write((char*)&mValue, sizeof(int));
}

void IntProperty::readProperty(QIODevice *target) {
    target->read((char*)&mMinValue, sizeof(int));
    target->read((char*)&mMaxValue, sizeof(int));
    target->read((char*)&mValue, sizeof(int));
}

void Key::writeKey(QIODevice *target) {
    target->write((char*)&mRelFrame, sizeof(int));
}

void Key::readKey(QIODevice *target) {
    target->read((char*)&mRelFrame, sizeof(int));
}

void PathContainer::writePathContainer(QIODevice *target) {
    int nPts = mElementsPos.count();
    target->write((char*)&nPts, sizeof(int)); // number pts
    foreach(const SkPoint &pos, mElementsPos) {
        SkScalar xT, yT;
        xT = pos.x(); yT = pos.y();
        target->write((char*)&xT, sizeof(SkScalar));
        target->write((char*)&yT, sizeof(SkScalar));
    }
    target->write((char*)&mPathClosed, sizeof(bool));
}

void PathContainer::readPathContainer(QIODevice *target) {
    int nPts;
    target->read((char*)&nPts, sizeof(int));
    for(int i = 0; i < nPts; i++) {
        SkScalar xT, yT;
        target->read((char*)&xT, sizeof(SkScalar));
        target->read((char*)&yT, sizeof(SkScalar));
        mElementsPos.append(SkPoint::Make(xT, yT));
    }
    target->read((char*)&mPathClosed, sizeof(bool));
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
    target->write((char*)&nNodes, sizeof(int));
    foreach(NodeSettings *nodeSettings, mNodeSettings) {
        target->write((char*)&nodeSettings->ctrlsMode, sizeof(CtrlsMode));
        target->write((char*)&nodeSettings->startEnabled, sizeof(bool));
        target->write((char*)&nodeSettings->endEnabled, sizeof(bool));
    }

    int nKeys = anim_mKeys.count();
    target->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->writeKey(target);
    }

    writePathContainer(target);
}

Key *VectorPathAnimator::readKey(QIODevice *target) {
    PathKey *newKey = new PathKey(this);

    newKey->readKey(target);
    return newKey;
}

void VectorPathAnimator::readProperty(QIODevice *target) {
    int nNodes;
    target->read((char*)&nNodes, sizeof(int));
    for(int i = 0; i < nNodes; i++) {
        NodeSettings nodeSettings;
        target->read((char*)&nodeSettings.ctrlsMode, sizeof(CtrlsMode));
        target->read((char*)&nodeSettings.startEnabled, sizeof(bool));
        target->read((char*)&nodeSettings.endEnabled, sizeof(bool));
        insertNodeSettingsForNodeId(i, nodeSettings);
    }

    int nKeys;
    target->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        anim_appendKey(readKey(target),
                       false,
                       false);
    }

    readPathContainer(target);
    updateNodePointsFromElements();
}

void QrealKey::writeKey(QIODevice *target) {
    Key::writeKey(target);
    target->write((char*)&mValue, sizeof(qreal));

    target->write((char*)&mStartEnabled, sizeof(bool));
    target->write((char*)&mStartFrame, sizeof(qreal));
    target->write((char*)&mStartValue, sizeof(qreal));

    target->write((char*)&mEndEnabled, sizeof(bool));
    target->write((char*)&mEndFrame, sizeof(qreal));
    target->write((char*)&mEndValue, sizeof(qreal));
}

void QrealKey::readKey(QIODevice *target) {
    Key::readKey(target);
    target->read((char*)&mValue, sizeof(qreal));

    target->read((char*)&mStartEnabled, sizeof(bool));
    target->read((char*)&mStartFrame, sizeof(qreal));
    target->read((char*)&mStartValue, sizeof(qreal));

    target->read((char*)&mEndEnabled, sizeof(bool));
    target->read((char*)&mEndFrame, sizeof(qreal));
    target->read((char*)&mEndValue, sizeof(qreal));
}

void QrealAnimator::writeProperty(QIODevice *target) {
    int nKeys = anim_mKeys.count();
    target->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QrealKey*)key.get())->writeKey(target);
    }

    target->write((char*)&mCurrentValue, sizeof(qreal));

    bool hasRandomGenerator = !mRandomGenerator.isNull();
    target->write((char*)&hasRandomGenerator, sizeof(bool));
    if(hasRandomGenerator) {
        mRandomGenerator->writeProperty(target);
    }
}

Key *QrealAnimator::readKey(QIODevice *target) {
    QrealKey *newKey = new QrealKey(this);
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
    target->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        anim_appendKey(readKey(target), false, false);
    }

    qreal val;
    target->read((char*)&val, sizeof(qreal));
    qra_setCurrentValue(val);
    bool hasRandomGenerator;
    target->read((char*)&hasRandomGenerator, sizeof(bool));
    if(hasRandomGenerator) {
        RandomQrealGenerator *generator = new RandomQrealGenerator(0, 9999);
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
    target->write((char*)&mColorMode, sizeof(ColorMode));
    mVal1Animator->writeProperty(target);
    mVal2Animator->writeProperty(target);
    mVal3Animator->writeProperty(target);
    mAlphaAnimator->writeProperty(target);
}

void ColorAnimator::readProperty(QIODevice *target) {
    target->read((char*)&mColorMode, sizeof(ColorMode));
    setColorMode(mColorMode);
    mVal1Animator->readProperty(target);
    mVal2Animator->readProperty(target);
    mVal3Animator->readProperty(target);
    mAlphaAnimator->readProperty(target);
}

void QStringKey::writeQStringKey(QIODevice *target) {
    writeQString(target, mText);
}

void QStringKey::readQStringKey(QIODevice *target) {
    readQString(target, &mText);
}

void QStringAnimator::writeProperty(QIODevice *target) {
    int nKeys = anim_mKeys.count();
    target->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QStringKey*)key.get())->writeQStringKey(target);
    }
    writeQString(target, mCurrentText);
}

void QStringAnimator::readProperty(QIODevice *target) {
    int nKeys;
    target->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        QStringKey *newKey = new QStringKey("", 0, this);
        newKey->readQStringKey(target);
        anim_appendKey(newKey, false, false);
    }
    readQString(target, &mCurrentText);
}

void PixmapEffect::writeProperty(QIODevice *target) {
    target->write((char*)&mType, sizeof(PixmapEffectType));
    target->write((char*)&mVisible, sizeof(bool));
}

void PixmapEffect::readProperty(QIODevice *target) {
    target->read((char*)&mVisible, sizeof(bool));
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
    target->write((char*)&nEffects, sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PixmapEffect*)effect.data())->writeProperty(target);
    }
}

void EffectAnimators::readPixmapEffect(QIODevice *target) {
    PixmapEffectType typeT;
    target->read((char*)&typeT, sizeof(PixmapEffectType));
    PixmapEffect *effect = NULL;
    if(typeT == EFFECT_BLUR) {
        effect = new BlurEffect();
    } else if(typeT == EFFECT_SHADOW) {
        effect = new ShadowEffect();
    } else if(typeT == EFFECT_DESATURATE) {
        effect = new DesaturateEffect();
    } else if(typeT == EFFECT_COLORIZE) {
        effect = new ColorizeEffect();
    } else if(typeT == EFFECT_REPLACE_COLOR) {
        effect = new ReplaceColorEffect();
    } else if(typeT == EFFECT_BRIGHTNESS) {
        effect = new BrightnessEffect();
    } else if(typeT == EFFECT_CONTRAST) {
        effect = new ContrastEffect();
    } else if(typeT == EFFECT_MOTION_BLUR) {
        effect = new SampledMotionBlurEffect(mParentBox);
    } else {
        return;
    }
    effect->readProperty(target);
    addEffect(effect);
}

void EffectAnimators::readProperty(QIODevice *target) {
    int nEffects;
    target->read((char*)&nEffects, sizeof(int));
    for(int i = 0; i < nEffects; i++) {
        readPixmapEffect(target);
    }
}

void BasicTransformAnimator::writeProperty(QIODevice *target) {
    mPosAnimator->writeProperty(target);
    mScaleAnimator->writeProperty(target);
    mRotAnimator->writeProperty(target);
    updateRelativeTransform();
}

void BasicTransformAnimator::readProperty(QIODevice *target) {
    mPosAnimator->readProperty(target);
    mScaleAnimator->readProperty(target);
    mRotAnimator->readProperty(target);
    updateRelativeTransform();
}

void BoxTransformAnimator::writeProperty(QIODevice *target) {
    BasicTransformAnimator::writeProperty(target);
    mOpacityAnimator->writeProperty(target);
    mPivotAnimator->writeProperty(target);
    updateRelativeTransform();
}

void BoxTransformAnimator::readProperty(QIODevice *target) {
    BasicTransformAnimator::readProperty(target);
    mOpacityAnimator->readProperty(target);
    mPivotAnimator->readProperty(target);
    updateRelativeTransform();
}

void GradientPoints::writeProperty(QIODevice *target) {
    startPoint->writeProperty(target);
    endPoint->writeProperty(target);
}

void GradientPoints::readProperty(QIODevice *target) {
    startPoint->readProperty(target);
    endPoint->readProperty(target);
}

void Gradient::writeProperty(QIODevice *target) {
    target->write((char*)&mLoadId, sizeof(int));
    int nColors = mColors.count();
    target->write((char*)&nColors, sizeof(int));
    foreach(ColorAnimator *color, mColors) {
        color->writeProperty(target);
    }
}

void Gradient::readProperty(QIODevice *target) {
    target->read((char*)&mLoadId, sizeof(int));
    int nColors;
    target->read((char*)&nColors, sizeof(int));
    for(int i = 0; i < nColors; i++) {
        ColorAnimator *colorAnim = new ColorAnimator();
        colorAnim->readProperty(target);
        addColorToList(colorAnim, false);
    }
}

void StrokeSettings::writeProperty(QIODevice *target) {
    PaintSettings::writeProperty(target);
    mLineWidth->writeProperty(target);
    target->write((char*)&mCapStyle, sizeof(Qt::PenCapStyle));
    target->write((char*)&mJoinStyle, sizeof(Qt::PenJoinStyle));
    target->write((char*)&mOutlineCompositionMode,
               sizeof(QPainter::CompositionMode));
}

void StrokeSettings::readProperty(QIODevice *target) {
    PaintSettings::readProperty(target);
    mLineWidth->readProperty(target);
    target->read((char*)&mCapStyle, sizeof(Qt::PenCapStyle));
    target->read((char*)&mJoinStyle, sizeof(Qt::PenJoinStyle));
    target->read((char*)&mOutlineCompositionMode,
                sizeof(QPainter::CompositionMode));
}

void PaintSettings::writeProperty(QIODevice *target) {
    mGradientPoints->writeProperty(target);
    mColor->writeProperty(target);
    target->write((char*)&mPaintType, sizeof(PaintType));
    int gradId;
    if(mGradient == NULL) {
        gradId = -1;
    } else {
        gradId = mGradient->getLoadId();
    }
    target->write((char*)&mGradientLinear, sizeof(bool));
    target->write((char*)&gradId, sizeof(int));
}

void PaintSettings::readProperty(QIODevice *target) {
    mGradientPoints->readProperty(target);
    mColor->readProperty(target);
    target->read((char*)&mPaintType, sizeof(PaintType));
    int gradId;
    target->read((char*)&mGradientLinear, sizeof(bool));
    target->read((char*)&gradId, sizeof(int));
    if(gradId != -1) {
        mGradient = MainWindow::getInstance()->
                getLoadedGradientById(gradId)->ref<Gradient>();
    }
}

void DurationRectangle::writeDurationRectangle(QIODevice *target) {
    int minFrame = getMinFrame();
    int maxFrame = getMaxFrame();
    int framePos = getFramePos();
    target->write((char*)&minFrame, sizeof(int));
    target->write((char*)&maxFrame, sizeof(int));
    target->write((char*)&framePos, sizeof(int));
}

void DurationRectangle::readDurationRectangle(QIODevice *target) {
    int minFrame;
    int maxFrame;
    int framePos;
    target->read((char*)&minFrame, sizeof(int));
    target->read((char*)&maxFrame, sizeof(int));
    target->read((char*)&framePos, sizeof(int));
    setMinFrame(minFrame);
    setMaxFrame(maxFrame);
    setFramePos(framePos);
}

void FixedLenAnimationRect::writeDurationRectangle(QIODevice *target) {
    DurationRectangle::writeDurationRectangle(target);
    target->write((char*)&mBoundToAnimation, sizeof(bool));
    target->write((char*)&mSetMaxFrameAtLeastOnce, sizeof(bool));
    target->write((char*)&mMinAnimationFrame, sizeof(int));
    target->write((char*)&mMaxAnimationFrame, sizeof(int));
}

void FixedLenAnimationRect::readDurationRectangle(QIODevice *target) {
    DurationRectangle::readDurationRectangle(target);
    int minFrame;
    int maxFrame;
    target->read((char*)&mBoundToAnimation, sizeof(bool));
    target->read((char*)&mSetMaxFrameAtLeastOnce, sizeof(bool));
    target->read((char*)&minFrame, sizeof(int));
    target->read((char*)&maxFrame, sizeof(int));
    setMinAnimationFrame(minFrame);
    setMaxAnimationFrame(maxFrame);
}

void BoundingBox::writeBoundingBox(QIODevice *target) {
    target->write((char*)&mType, sizeof(BoundingBoxType));
    writeQString(target, prp_mName);
    target->write((char*)&mLoadId, sizeof(int));
    target->write((char*)&mPivotAutoAdjust, sizeof(bool));
    target->write((char*)&mVisible, sizeof(bool));
    target->write((char*)&mLocked, sizeof(bool));
    target->write((char*)&mBlendModeSk, sizeof(SkBlendMode));
    bool hasDurRect = mDurationRectangle != NULL;
    target->write((char*)&hasDurRect, sizeof(bool));

    if(hasDurRect) {
        mDurationRectangle->writeDurationRectangle(target);
    }

    mTransformAnimator->writeProperty(target);
    mEffectsAnimators->writeProperty(target);
}

void BoundingBox::readBoundingBox(QIODevice *target) {
    readQString(target, &prp_mName);
    target->read((char*)&mLoadId, sizeof(int));
    target->read((char*)&mPivotAutoAdjust, sizeof(bool));
    target->read((char*)&mVisible, sizeof(bool));
    target->read((char*)&mLocked, sizeof(bool));
    target->read((char*)&mBlendModeSk, sizeof(SkBlendMode));
    bool hasDurRect;
    target->read((char*)&hasDurRect, sizeof(bool));

    if(hasDurRect) {
        if(mDurationRectangle == NULL) createDurationRectangle();
        mDurationRectangle->readDurationRectangle(target);
    }

    mTransformAnimator->readProperty(target);
    mEffectsAnimators->readProperty(target);
    BoundingBox::addLoadedBox(this);
}

void BoundingBox::writeBoundingBoxDataForLink(QIODevice *target) {
    target->write((char*)&mType, sizeof(BoundingBoxType));
    writeQString(target, prp_mName);
    target->write((char*)&mLoadId, sizeof(int));
    target->write((char*)&mPivotAutoAdjust, sizeof(bool));
    target->write((char*)&mVisible, sizeof(bool));
    target->write((char*)&mLocked, sizeof(bool));
    target->write((char*)&mBlendModeSk, sizeof(SkBlendMode));
//    bool hasDurRect = mDurationRectangle != NULL;
//    target->write((char*)&hasDurRect, sizeof(bool));

//    if(hasDurRect) {
//        mDurationRectangle->writeDurationRectangle(target);
//    }

    mTransformAnimator->writeProperty(target);
    mEffectsAnimators->writeProperty(target);
}

void BoundingBox::readBoundingBoxDataForLink(QIODevice *target) {
    readQString(target, &prp_mName);
    target->read((char*)&mLoadId, sizeof(int));
    target->read((char*)&mPivotAutoAdjust, sizeof(bool));
    target->read((char*)&mVisible, sizeof(bool));
    target->read((char*)&mLocked, sizeof(bool));
    target->read((char*)&mBlendModeSk, sizeof(SkBlendMode));
//    bool hasDurRect;
//    target->read((char*)&hasDurRect, sizeof(bool));

//    if(hasDurRect) {
//        if(mDurationRectangle == NULL) createDurationRectangle();
//        mDurationRectangle->readDurationRectangle(target);
//    }

    mTransformAnimator->readProperty(target);
    mEffectsAnimators->readProperty(target);
    BoundingBox::addLoadedBox(this);
}

void PathEffect::writeProperty(QIODevice *target) {
    target->write((char*)&mPathEffectType, sizeof(PathEffectType));
    target->write((char*)&mVisible, sizeof(bool));
}

void PathEffect::readProperty(QIODevice *target) {
    target->read((char*)&mVisible, sizeof(bool));
}

void DisplacePathEffect::writeProperty(QIODevice *target) {
    PathEffect::writeProperty(target);
    mSegLength->writeProperty(target);
    mMaxDev->writeProperty(target);
    mSmoothness->writeProperty(target);
}

void DisplacePathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mSegLength->readProperty(target);
    mMaxDev->readProperty(target);
    mSmoothness->readProperty(target);
}

void DuplicatePathEffect::writeProperty(QIODevice *target) {
    PathEffect::writeProperty(target);
    mTranslation->writeProperty(target);
}

void DuplicatePathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mTranslation->readProperty(target);
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
    BoundingBox *targetBox = mTarget.data();
    int targetId;
    if(targetBox == NULL) {
        targetId = -1;
    } else {
        targetId = targetBox->getLoadId();
        if(targetId < 0) {
            targetId = BoundingBox::getLoadedBoxesCount();
            targetBox->setBoxLoadId(targetId);
            BoundingBox::addLoadedBox(targetBox);
        }
    }
    target->write((char*)&targetId, sizeof(int));
}

void BoxTargetProperty::readProperty(QIODevice *target) {
    int targetId;
    target->read((char*)&targetId, sizeof(int));
    BoundingBox *targetBox = BoundingBox::getLoadedBoxById(targetId);
    if(targetBox == NULL && targetId >= 0) {
        BoundingBox::addFunctionWaitingForBoxLoad(
                    new BoxTargetPropertyWaitingForBoxLoad(targetId, this) );
    } else {
        setTarget(targetBox);
    }
}

void SumPathEffect::writeProperty(QIODevice *target) {
    PathEffect::writeProperty(target);
    mBoxTarget->writeProperty(target);
}

void SumPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mBoxTarget->readProperty(target);
}

void PathEffectAnimators::writeProperty(QIODevice *target) {   
    int nEffects = ca_mChildAnimators.count();
    target->write((char*)&nEffects, sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PathEffect*)effect.data())->writeProperty(target);
    }
}

void PathEffectAnimators::readPathEffect(QIODevice *target) {
    PathEffectType typeT;
    target->read((char*)&typeT, sizeof(PathEffectType));
    if(typeT == DISPLACE_PATH_EFFECT) {
        DisplacePathEffect *displaceEffect =
                new DisplacePathEffect(mIsOutline);
        displaceEffect->readProperty(target);
        addEffect(displaceEffect);
    } else if(typeT == DUPLICATE_PATH_EFFECT) {
        DuplicatePathEffect *duplicateEffect =
                new DuplicatePathEffect(mIsOutline);
        duplicateEffect->readProperty(target);
        addEffect(duplicateEffect);
    } else if(typeT == SUM_PATH_EFFECT) {
        SumPathEffect *sumEffect =
                new SumPathEffect((PathBox*)mParentBox, mIsOutline);
        sumEffect->readProperty(target);
        addEffect(sumEffect);
    } else if(typeT == GROUP_SUM_PATH_EFFECT) {
        GroupLastPathSumPathEffect *sumEffect =
                new GroupLastPathSumPathEffect((BoxesGroup*)mParentBox,
                                               mIsOutline);
        sumEffect->readProperty(target);
        addEffect(sumEffect);
    }
}

void PathEffectAnimators::readProperty(QIODevice *target) {
    int nEffects;
    target->read((char*)&nEffects, sizeof(int));
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
    int nEmitters = mEmitters.count();
    target->write((char*)&nEmitters, sizeof(int));
    foreach(ParticleEmitter *emitter, mEmitters) {
        emitter->writeProperty(target);
    }
}

void ParticleBox::readBoundingBox(QIODevice *target) {
    int nEmitters;
    target->read((char*)&nEmitters, sizeof(int));
    for(int i = 0; i < nEmitters; i++) {
        ParticleEmitter *emitter = new ParticleEmitter(this);
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
    readQString(target, &path);
    setFilePath(path);
}

void Circle::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mHorizontalRadiusPoint->writeProperty(target);
    mVerticalRadiusPoint->writeProperty(target);
}

void Circle::readBoundingBox(QIODevice *target) {
    PathBox::readBoundingBox(target);
    mHorizontalRadiusPoint->readProperty(target);
    mVerticalRadiusPoint->readProperty(target);
}

void Rectangle::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mRadiusPoint->writeProperty(target);
    mTopLeftPoint->writeProperty(target);
    mBottomRightPoint->writeProperty(target);
}

void Rectangle::readBoundingBox(QIODevice *target) {
    PathBox::readBoundingBox(target);
    mRadiusPoint->readProperty(target);
    mTopLeftPoint->readProperty(target);
    mBottomRightPoint->readProperty(target);
}

void VideoBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    writeQString(target, mSrcFilePath);
}

void VideoBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    readQString(target, &path);
    setFilePath(path);
}

bool Tile::writeTile(QIODevice *target) {
    return target->write((char*)mData, TILE_DIM*TILE_DIM*4*sizeof(uchar)) > 0;
}

void Tile::readTile(QIODevice *target) {
    target->read((char*)mData, TILE_DIM*TILE_DIM*4*sizeof(uchar));
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
    target->write((char*)&mWidth, sizeof(ushort));
    target->write((char*)&mHeight, sizeof(ushort));
    bool noDataInMemory = !mDataStoredInTmpFile && mNoDataInMemory;
    target->write((char*)&noDataInMemory, sizeof(bool));
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
    target->read((char*)&width, sizeof(ushort));
    target->read((char*)&height, sizeof(ushort));
    target->read((char*)&mNoDataInMemory, sizeof(bool));
    setSize(width, height);
    if(mNoDataInMemory) return;
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->readTile(target);
        }
    }
}

void Surface::writeSurface(QIODevice *target) {
    target->write((char*)&mWidth, sizeof(ushort));
    target->write((char*)&mHeight, sizeof(ushort));
    mCurrentTiles->writeTilesDataFromMemoryOrTmp(target);
}

void Surface::readSurface(QIODevice *target) {
    target->read((char*)&mWidth, sizeof(ushort));
    target->read((char*)&mHeight, sizeof(ushort));
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
    target->write((char*)&mIsDraft, sizeof(bool));
    target->write((char*)&mWidth, sizeof(ushort));
    target->write((char*)&mHeight, sizeof(ushort));
    int nKeys = anim_mKeys.count();
    target->write((char*)&nKeys, sizeof(int));
    if(nKeys == 0) {
        mCurrentTiles->writeTilesDataFromMemoryOrTmp(target);
    } else {
        foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
            ((SurfaceKey*)key.get())->writeKey(target);
        }
    }
}

Key *AnimatedSurface::readKey(QIODevice *target) {
    SurfaceKey *newKey = new SurfaceKey(this);
    newKey->setTiles(new TilesData(0,
                                   0, true));
    newKey->readKey(target);
    return newKey;
}

void AnimatedSurface::readProperty(QIODevice *target) {
    target->read((char*)&mIsDraft, sizeof(bool));
    target->read((char*)&mWidth, sizeof(ushort));
    target->read((char*)&mHeight, sizeof(ushort));
    int nKeys;
    target->read((char*)&nKeys, sizeof(int));
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
    mTopLeftPoint->writeProperty(target);
    mBottomRightPoint->writeProperty(target);
    mMainHandler->writeProperty(target);
}

void PaintBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    mTopLeftPoint->readProperty(target);
    mBottomRightPoint->readProperty(target);
    finishSizeSetup();
    mMainHandler->readProperty(target);
}

void ImageSequenceBox::writeBoundingBox(QIODevice *target) {
    BoundingBox::writeBoundingBox(target);
    int nFrames = mListOfFrames.count();
    target->write((char*)&nFrames, sizeof(int));
    foreach(const QString &frame, mListOfFrames) {
        writeQString(target, frame);
    }
}

void ImageSequenceBox::readBoundingBox(QIODevice *target) {
    BoundingBox::readBoundingBox(target);
    int nFrames;
    target->read((char*)&nFrames, sizeof(int));
    for(int i = 0; i < nFrames; i++) {
        QString frame;
        readQString(target, &frame);
        mListOfFrames << frame;
    }
}

void TextBox::writeBoundingBox(QIODevice *target) {
    PathBox::writeBoundingBox(target);
    mText->writeProperty(target);
    target->write((char*)&mAlignment, sizeof(Qt::Alignment));
    qreal fontSize = mFont.pointSizeF();
    QString fontFamily = mFont.family();
    QString fontStyle = mFont.styleName();
    target->write((char*)&fontSize, sizeof(qreal));
    writeQString(target, fontFamily);
    writeQString(target, fontStyle);
}

void TextBox::readBoundingBox(QIODevice *target) {
    PathBox::readBoundingBox(target);
    mText->readProperty(target);
    target->read((char*)&mAlignment, sizeof(Qt::Alignment));
    qreal fontSize;
    QString fontFamily;
    QString fontStyle;
    target->read((char*)&fontSize, sizeof(qreal));
    readQString(target, &fontFamily);
    readQString(target, &fontStyle);
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
    target->write((char*)&nChildBoxes, sizeof(int));
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
        child->writeBoundingBox(target);
    }
}

void BoxesGroup::readChildBoxes(QIODevice *target) {
    int nChildBoxes;
    target->read((char*)&nChildBoxes, sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        BoundingBox *box;
        BoundingBoxType boxType;
        target->read((char*)&boxType, sizeof(BoundingBoxType));
        if(boxType == TYPE_VECTOR_PATH) {
            box = new VectorPath();
        } else if(boxType == TYPE_IMAGE) {
            box = new ImageBox();
        } else if(boxType == TYPE_TEXT) {
            box = new TextBox();
        } else if(boxType == TYPE_VIDEO) {
            box = new VideoBox();
        } else if(boxType == TYPE_PARTICLES) {
            box = new ParticleBox();
        } else if(boxType == TYPE_RECTANGLE) {
            box = new Rectangle();
        } else if(boxType == TYPE_CIRCLE) {
            box = new Circle();
        } else if(boxType == TYPE_GROUP) {
            box = new BoxesGroup();
        } else if(boxType == TYPE_PAINT) {
            box = new PaintBox();
        } else if(boxType == TYPE_IMAGESQUENCE) {
            box = new ImageSequenceBox();
        } else if(boxType == TYPE_INTERNAL_LINK) {
            box = new InternalLinkBox(NULL);
        } else if(boxType == TYPE_EXTERNAL_LINK) {
            box = new ExternalLinkBox();
        } else if(boxType == TYPE_INTERNAL_LINK_CANVAS) {
            box = new InternalLinkCanvas(NULL);
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
    target->write((char*)&nPaths, sizeof(int));
    foreach(VectorPathAnimator *pathAnimator, mSinglePaths) {
        pathAnimator->writeProperty(target);
    }
}

void PathAnimator::readVectorPathAnimator(QIODevice *target) {
    VectorPathAnimator *pathAnimator = new VectorPathAnimator(this);
    pathAnimator->readProperty(target);
    addSinglePathAnimator(pathAnimator, false);
}

QMatrix PathAnimator::getCombinedTransform() {
    if(mParentBox == NULL) return QMatrix();
    return mParentBox->getCombinedTransform();
}

void PathAnimator::readProperty(QIODevice *target) {
    int nPaths;
    target->read((char*)&nPaths, sizeof(int));
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
    target->write((char*)&mWidth, sizeof(int));
    target->write((char*)&mHeight, sizeof(int));
    target->write((char*)&mFps, sizeof(qreal));
    target->write((char*)&mMaxFrame, sizeof(int));
    target->write((char*)&mCanvasTransformMatrix, sizeof(QMatrix));
}

void Canvas::readBoundingBox(QIODevice *target) {
    target->read((char*)&mType, sizeof(BoundingBoxType));
    BoxesGroup::readBoundingBox(target);
    target->read((char*)&mWidth, sizeof(int));
    target->read((char*)&mHeight, sizeof(int));
    target->read((char*)&mFps, sizeof(qreal));
    target->read((char*)&mMaxFrame, sizeof(int));
    target->read((char*)&mCanvasTransformMatrix, sizeof(QMatrix));
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
}

void GradientWidget::writeGradients(QIODevice *target) {
    int nGradients = mGradients.count();
    target->write((char*)&nGradients, sizeof(int));
    foreach(const QSharedPointer<Gradient> &gradient, mGradients) {
        gradient->writeProperty(target);
    }
}

void GradientWidget::readGradients(QIODevice *target) {
    int nGradients;
    target->read((char*)&nGradients, sizeof(int));
    for(int i = 0; i < nGradients; i++) {
        Gradient *gradient = new Gradient();
        gradient->readProperty(target);
        addGradientToList(gradient);
        MainWindow::getInstance()->addLoadedGradient(gradient);
    }
}

void Brush::writeBrush(QIODevice *write) {
    writeQString(write, brush_name);
    writeQString(write, collection_name);

    for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSettingInfo setting_info = brush_settings_info[i];
        qreal value = getSettingVal(setting_info.setting);
        write->write((const char *)&value, sizeof(qreal));
    }
}

void Brush::readBrush(QIODevice *read) {
    readQString(read, &brush_name);
    readQString(read, &collection_name);

    for(uchar i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        qreal value;
        read->read((char *)&value, sizeof(qreal));
        BrushSettingInfo setting_info = brush_settings_info[i];
        setSetting(setting_info.setting, value);
    }
}

void Color::readColor(QIODevice *read) {
    QColor qcolT;
    read->read((char*)&qcolT, sizeof(QColor));
    setQColor(qcolT);
}

void Color::writeColor(QIODevice *write) {
    write->write((const char*)&qcol, sizeof(QColor));
}

void CanvasWindow::writeCanvases(QIODevice *target) {
    int nCanvases = mCanvasList.count();
    target->write((char*)&nCanvases, sizeof(int));
    int currentCanvasId = -1;
    int boxLoadId = 0;
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
        boxLoadId = canvas->setBoxLoadId(boxLoadId);
        canvas->writeBoundingBox(target);
        if(canvas == mCurrentCanvas) {
            currentCanvasId = mCurrentCanvas->getLoadId();
        }
    }
    target->write((char*)&currentCanvasId, sizeof(int));
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
        canvas->clearBoxLoadId();
    }
}

void CanvasWindow::readCanvases(QIODevice *target) {
    int nCanvases;
    target->read((char*)&nCanvases, sizeof(int));
    for(int i = 0; i < nCanvases; i++) {
        FillStrokeSettingsWidget *fillStrokeSettingsWidget =
                MainWindow::getInstance()->getFillStrokeSettings();
        Canvas *canvas = new Canvas(fillStrokeSettingsWidget, this);
        canvas->readBoundingBox(target);
        MainWindow::getInstance()->addCanvas(canvas);
    }
    int currentCanvasId;
    target->read((char*)&currentCanvasId, sizeof(int));
    setCurrentCanvas((Canvas*)BoundingBox::getLoadedBoxById(currentCanvasId));
}

void MainWindow::loadAVFile(const QString &path) {
    QFile target(path);
    if(target.open(QIODevice::ReadOnly) ) {
        FileFooter footer;
        footer.read(&target);
        if(footer.combatybilityMode() ==
                FileFooter::CompatybilityMode::Compatible) {
            QColor qcolT;
            target.read((char*)&qcolT, sizeof(QColor));
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
        target.write((char*)&color, sizeof(QColor));

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
