#include "avfileio.h"
#include <fstream>
#include "Animators/PathAnimators/pathkey.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/coloranimator.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Animators/qstringanimator.h"
#include "Animators/transformanimator.h"
#include "Animators/paintsettings.h"
#include "Animators/pathanimator.h"
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

void writeQString(QFile *file,
                 const QString &strToWrite) {
    int nChars = strToWrite.length();
    file->write((char*)&nChars, sizeof(int));
    file->write((char*)strToWrite.utf16(), nChars*sizeof(ushort));
}

void readQString(QFile *file,
                QString *targetStr) {
    int nChars;
    file->read((char*)&nChars, sizeof(int));
    ushort *chars = new ushort[nChars];

    file->read((char*)chars, nChars*sizeof(ushort));
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

    void write(QFile *file) {
        file->write((char*)this, sizeof(FileFooter));
    }

    void read(QFile *file) {
        file->seek(file->size() - sizeof(FileFooter));
        file->read((char*)this, sizeof(FileFooter));
        file->seek(0);
    }


};

void BoolProperty::writeBoolProperty(QFile *file) {
    file->write((char*)&mValue, sizeof(bool));
}

void BoolProperty::readBoolProperty(QFile *file) {
    file->read((char*)&mValue, sizeof(bool));
}

void Key::writeKey(QFile *file) {
    file->write((char*)&mRelFrame, sizeof(int));
}

void Key::readKey(QFile *file) {
    file->read((char*)&mRelFrame, sizeof(int));
}

void PathContainer::writePathContainer(QFile *file) {
    int nPts = mElementsPos.count();
    file->write((char*)&nPts, sizeof(int)); // number pts
    foreach(const SkPoint &pos, mElementsPos) {
        SkScalar xT, yT;
        xT = pos.x(); yT = pos.y();
        file->write((char*)&xT, sizeof(SkScalar));
        file->write((char*)&yT, sizeof(SkScalar));
    }
    file->write((char*)&mPathClosed, sizeof(bool));
}

void PathContainer::readPathContainer(QFile *file) {
    int nPts;
    file->read((char*)&nPts, sizeof(int));
    for(int i = 0; i < nPts; i++) {
        SkScalar xT, yT;
        file->read((char*)&xT, sizeof(SkScalar));
        file->read((char*)&yT, sizeof(SkScalar));
        mElementsPos.append(SkPoint::Make(xT, yT));
    }
    file->read((char*)&mPathClosed, sizeof(bool));
    mPathUpdateNeeded = true;
}

void PathKey::writePathKey(QFile *file) {
    writeKey(file);
    writePathContainer(file);
}

void PathKey::readPathKey(QFile *file) {
    readKey(file);
    readPathContainer(file);
}

void VectorPathAnimator::writeVectorPathAnimator(QFile *file) {
    int nNodes = mNodeSettings.count();
    file->write((char*)&nNodes, sizeof(int));
    foreach(NodeSettings *nodeSettings, mNodeSettings) {
        file->write((char*)&nodeSettings->ctrlsMode, sizeof(CtrlsMode));
        file->write((char*)&nodeSettings->startEnabled, sizeof(bool));
        file->write((char*)&nodeSettings->endEnabled, sizeof(bool));
    }

    int nKeys = anim_mKeys.count();
    file->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->writePathKey(file);
    }

    writePathContainer(file);
}

void VectorPathAnimator::readVectorPathAnimator(QFile *file) {
    int nNodes;
    file->read((char*)&nNodes, sizeof(int));
    for(int i = 0; i < nNodes; i++) {
        NodeSettings nodeSettings;
        file->read((char*)&nodeSettings.ctrlsMode, sizeof(CtrlsMode));
        file->read((char*)&nodeSettings.startEnabled, sizeof(bool));
        file->read((char*)&nodeSettings.endEnabled, sizeof(bool));
        insertNodeSettingsForNodeId(i, nodeSettings);
    }

    int nKeys;
    file->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        PathKey *newKey = new PathKey(this);

        newKey->readPathKey(file);

        anim_appendKey(newKey,
                       false,
                       false);
    }

    readPathContainer(file);
}

void QrealKey::writeQrealKey(QFile *file) {
    writeKey(file);
    file->write((char*)&mValue, sizeof(qreal));

    file->write((char*)&mStartEnabled, sizeof(bool));
    file->write((char*)&mStartFrame, sizeof(qreal));
    file->write((char*)&mStartValue, sizeof(qreal));

    file->write((char*)&mEndEnabled, sizeof(bool));
    file->write((char*)&mEndFrame, sizeof(qreal));
    file->write((char*)&mEndValue, sizeof(qreal));
}

void QrealKey::readQrealKey(QFile *file) {
    readKey(file);
    file->read((char*)&mValue, sizeof(qreal));

    file->read((char*)&mStartEnabled, sizeof(bool));
    file->read((char*)&mStartFrame, sizeof(qreal));
    file->read((char*)&mStartValue, sizeof(qreal));

    file->read((char*)&mEndEnabled, sizeof(bool));
    file->read((char*)&mEndFrame, sizeof(qreal));
    file->read((char*)&mEndValue, sizeof(qreal));
}

void QrealAnimator::writeQrealAnimator(QFile *file) {
    int nKeys = anim_mKeys.count();
    file->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QrealKey*)key.get())->writeQrealKey(file);
    }

    file->write((char*)&mCurrentValue, sizeof(qreal));
}

void QrealAnimator::readQrealAnimator(QFile *file) {
    int nKeys;
    file->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        QrealKey *newKey = new QrealKey(this);
        newKey->readQrealKey(file);
        anim_appendKey(newKey, false, false);
    }

    file->read((char*)&mCurrentValue, sizeof(qreal));
}

void QPointFAnimator::writeQPointFAnimator(QFile *file) {
    mXAnimator->writeQrealAnimator(file);
    mYAnimator->writeQrealAnimator(file);
}

void QPointFAnimator::readQPointFAnimator(QFile *file) {
    mXAnimator->readQrealAnimator(file);
    mYAnimator->readQrealAnimator(file);
}

void ColorAnimator::writeColorAnimator(QFile *file) {
    file->write((char*)&mColorMode, sizeof(ColorMode));
    mVal1Animator->writeQrealAnimator(file);
    mVal2Animator->writeQrealAnimator(file);
    mVal3Animator->writeQrealAnimator(file);
    mAlphaAnimator->writeQrealAnimator(file);
}

void ColorAnimator::readColorAnimator(QFile *file) {
    file->read((char*)&mColorMode, sizeof(ColorMode));
    setColorMode(mColorMode);
    mVal1Animator->readQrealAnimator(file);
    mVal2Animator->readQrealAnimator(file);
    mVal3Animator->readQrealAnimator(file);
    mAlphaAnimator->readQrealAnimator(file);
}

void QStringKey::writeQStringKey(QFile *file) {
    writeQString(file, mText);
}

void QStringKey::readQStringKey(QFile *file) {
    readQString(file, &mText);
}

void QStringAnimator::writeQStringAnimator(QFile *file) {
    int nKeys = anim_mKeys.count();
    file->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QStringKey*)key.get())->writeQStringKey(file);
    }
    writeQString(file, mCurrentText);
}

void QStringAnimator::readQStringAnimator(QFile *file) {
    int nKeys;
    file->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        QStringKey *newKey = new QStringKey("", 0, this);
        newKey->readQStringKey(file);
        anim_appendKey(newKey, false, false);
    }
    readQString(file, &mCurrentText);
}

void PixmapEffect::writePixmapEffect(QFile *file) {
    file->write((char*)&mType, sizeof(PixmapEffectType));
}

void BlurEffect::readBlurEffect(QFile *file) {
    mHighQuality->readBoolProperty(file);
    mBlurRadius->readQrealAnimator(file);
}

void BlurEffect::writePixmapEffect(QFile *file) {
    PixmapEffect::writePixmapEffect(file);
    mHighQuality->writeBoolProperty(file);
    mBlurRadius->writeQrealAnimator(file);
}

void EffectAnimators::writeEffectAnimators(QFile *file) {
    int nEffects = ca_mChildAnimators.count();
    file->write((char*)&nEffects, sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PixmapEffect*)effect.data())->writePixmapEffect(file);
    }
}

void EffectAnimators::readEffectAnimators(QFile *file) {
    int nEffects;
    file->read((char*)&nEffects, sizeof(int));
    for(int i = 0; i < nEffects; i++) {
        PixmapEffectType typeT;
        file->read((char*)&typeT, sizeof(PixmapEffectType));
        if(typeT == EFFECT_BLUR) {
            BlurEffect *blurEffect = new BlurEffect();
            blurEffect->readBlurEffect(file);
            addEffect(blurEffect);
        }
    }
}

void BasicTransformAnimator::writeBasicTransformAnimator(QFile *file) {
    mPosAnimator->writeQPointFAnimator(file);
    mScaleAnimator->writeQPointFAnimator(file);
    mRotAnimator->writeQrealAnimator(file);
    updateRelativeTransform();
}

void BasicTransformAnimator::readBasicTransformAnimator(QFile *file) {
    mPosAnimator->readQPointFAnimator(file);
    mScaleAnimator->readQPointFAnimator(file);
    mRotAnimator->readQrealAnimator(file);
    updateRelativeTransform();
}

void BoxTransformAnimator::writeBoxTransformAnimator(QFile *file) {
    writeBasicTransformAnimator(file);
    mOpacityAnimator->writeQrealAnimator(file);
    mPivotAnimator->writeQPointFAnimator(file);
    updateRelativeTransform();
}

void BoxTransformAnimator::readBoxTransformAnimator(QFile *file) {
    readBasicTransformAnimator(file);
    mOpacityAnimator->readQrealAnimator(file);
    mPivotAnimator->readQPointFAnimator(file);
    updateRelativeTransform();
}

void GradientPoints::writeGradientPoints(QFile *file) {
    startPoint->writeQPointFAnimator(file);
    endPoint->writeQPointFAnimator(file);
}

void GradientPoints::readGradientPoints(QFile *file) {
    startPoint->readQPointFAnimator(file);
    endPoint->readQPointFAnimator(file);
}

void Gradient::writeGradient(QFile *file) {
    file->write((char*)&mLoadId, sizeof(int));
    int nColors = mColors.count();
    file->write((char*)&nColors, sizeof(int));
    foreach(ColorAnimator *color, mColors) {
        color->writeColorAnimator(file);
    }
}

void Gradient::readGradient(QFile *file) {
    file->read((char*)&mLoadId, sizeof(int));
    int nColors;
    file->read((char*)&nColors, sizeof(int));
    for(int i = 0; i < nColors; i++) {
        ColorAnimator *colorAnim = new ColorAnimator();
        colorAnim->readColorAnimator(file);
        addColorToList(colorAnim, false);
    }
}

void StrokeSettings::writeStrokeSettings(QFile *file) {
    writePaintSettings(file);
    mLineWidth->writeQrealAnimator(file);
    file->write((char*)&mCapStyle, sizeof(Qt::PenCapStyle));
    file->write((char*)&mJoinStyle, sizeof(Qt::PenJoinStyle));
    file->write((char*)&mOutlineCompositionMode,
               sizeof(QPainter::CompositionMode));
}

void StrokeSettings::readStrokeSettings(QFile *file) {
    readPaintSettings(file);
    mLineWidth->readQrealAnimator(file);
    file->read((char*)&mCapStyle, sizeof(Qt::PenCapStyle));
    file->read((char*)&mJoinStyle, sizeof(Qt::PenJoinStyle));
    file->read((char*)&mOutlineCompositionMode,
                sizeof(QPainter::CompositionMode));
}

void PaintSettings::writePaintSettings(QFile *file) {
    mGradientPoints->writeGradientPoints(file);
    mColor->writeColorAnimator(file);
    file->write((char*)&mPaintType, sizeof(PaintType));
    int gradId;
    if(mGradient == NULL) {
        gradId = -1;
    } else {
        gradId = mGradient->getLoadId();
    }
    file->write((char*)&mGradientLinear, sizeof(bool));
    file->write((char*)&gradId, sizeof(int));
}

void PaintSettings::readPaintSettings(QFile *file) {
    mGradientPoints->readGradientPoints(file);
    mColor->readColorAnimator(file);
    file->read((char*)&mPaintType, sizeof(PaintType));
    int gradId;
    file->read((char*)&mGradientLinear, sizeof(bool));
    file->read((char*)&gradId, sizeof(int));
    if(gradId != -1) {
        mGradient = MainWindow::getInstance()->
                getLoadedGradientById(gradId)->ref<Gradient>();
    }
}

void DurationRectangle::writeDurationRectangle(QFile *file) {
    int minFrame = getMinFrame();
    int maxFrame = getMaxFrame();
    int framePos = getFramePos();
    file->write((char*)&minFrame, sizeof(int));
    file->write((char*)&maxFrame, sizeof(int));
    file->write((char*)&framePos, sizeof(int));
}

void DurationRectangle::readDurationRectangle(QFile *file) {
    int minFrame;
    int maxFrame;
    int framePos;
    file->read((char*)&minFrame, sizeof(int));
    file->read((char*)&maxFrame, sizeof(int));
    file->read((char*)&framePos, sizeof(int));
    setMinFrame(minFrame);
    setMaxFrame(maxFrame);
    setFramePos(framePos);
}

void BoundingBox::writeBoundingBox(QFile *file) {
    file->write((char*)&mType, sizeof(BoundingBoxType));
    writeQString(file, prp_mName);
    file->write((char*)&mLoadId, sizeof(int));
    file->write((char*)&mPivotAutoAdjust, sizeof(bool));
    file->write((char*)&mVisible, sizeof(bool));
    file->write((char*)&mLocked, sizeof(bool));
    file->write((char*)&mBlendModeSk, sizeof(SkBlendMode));
    bool hasDurRect = mDurationRectangle != NULL;
    file->write((char*)&hasDurRect, sizeof(bool));

    if(hasDurRect) {
        mDurationRectangle->writeDurationRectangle(file);
    }

    mTransformAnimator->writeBoxTransformAnimator(file);
}

void BoundingBox::readBoundingBox(QFile *file) {
    readQString(file, &prp_mName);
    file->read((char*)&mLoadId, sizeof(int));
    file->read((char*)&mPivotAutoAdjust, sizeof(bool));
    file->read((char*)&mVisible, sizeof(bool));
    file->read((char*)&mLocked, sizeof(bool));
    file->read((char*)&mBlendModeSk, sizeof(SkBlendMode));
    bool hasDurRect;
    file->read((char*)&hasDurRect, sizeof(bool));

    if(hasDurRect) {
        if(mDurationRectangle == NULL) createDurationRectangle();
        mDurationRectangle->readDurationRectangle(file);
    }

    mTransformAnimator->readBoxTransformAnimator(file);
    BoundingBox::addLoadedBox(this);
}

void PathEffect::writePathEffect(QFile *file) {
    file->write((char*)&mPathEffectType, sizeof(PathEffectType));
}

void DisplacePathEffect::writePathEffect(QFile *file) {
    PathEffect::writePathEffect(file);
    mSegLength->writeQrealAnimator(file);
    mMaxDev->writeQrealAnimator(file);
    mSmoothness->writeQrealAnimator(file);
}

void DisplacePathEffect::readDisplacePathEffect(QFile *file) {
    mSegLength->readQrealAnimator(file);
    mMaxDev->readQrealAnimator(file);
    mSmoothness->readQrealAnimator(file);
}

void DuplicatePathEffect::writePathEffect(QFile *file) {
    PathEffect::writePathEffect(file);
    mTranslation->writeQPointFAnimator(file);
}

void DuplicatePathEffect::readDuplicatePathEffect(QFile *file) {
    mTranslation->readQPointFAnimator(file);
}

void BoxTargetProperty::writeBoxTargetProperty(QFile *file) {
    BoundingBox *targetBox = mTarget.data();
    int targetId;
    if(targetBox == NULL) {
        targetId = -1;
    } else {
        targetId = targetBox->getLoadId();
    }
    file->write((char*)&targetId, sizeof(int));
}

void BoxTargetProperty::readBoxTargetProperty(QFile *file) {
    int targetId;
    file->read((char*)&targetId, sizeof(int));
    BoundingBox *targetBox = BoundingBox::getLoadedBoxById(targetId);
    if(targetBox == NULL && targetId >= 0) {
        BoundingBox::addFunctionWaitingForBoxLoad(
                    new SumPathEffectForBoxLoad(targetId, this) );
    } else {
        setTarget(targetBox);
    }
}

void SumPathEffect::writePathEffect(QFile *file) {
    PathEffect::writePathEffect(file);
    mBoxTarget->writeBoxTargetProperty(file);
}

void SumPathEffect::readSumPathEffect(QFile *file) {
    mBoxTarget->readBoxTargetProperty(file);
}

void PathEffectAnimators::writePathEffectAnimators(QFile *file) {
    int nEffects = ca_mChildAnimators.count();
    file->write((char*)&nEffects, sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PathEffect*)effect.data())->writePathEffect(file);
    }
}

void PathEffectAnimators::readPathEffectAnimators(QFile *file) {
    int nEffects;
    file->read((char*)&nEffects, sizeof(int));
    for(int i = 0; i < nEffects; i++) {
        PathEffectType typeT;
        file->read((char*)&typeT, sizeof(PathEffectType));
        if(typeT == DISPLACE_PATH_EFFECT) {
            DisplacePathEffect *displaceEffect = new DisplacePathEffect();
            displaceEffect->readDisplacePathEffect(file);
            addEffect(displaceEffect);
        } else if(typeT == DUPLICATE_PATH_EFFECT) {
            DuplicatePathEffect *duplicateEffect = new DuplicatePathEffect();
            duplicateEffect->readDuplicatePathEffect(file);
            addEffect(duplicateEffect);
        } else if(typeT == SUM_PATH_EFFECT) {
            SumPathEffect *sumEffect = new SumPathEffect(NULL);
            sumEffect->readSumPathEffect(file);
            addEffect(sumEffect);
        }
    }
}

void PathBox::writeBoundingBox(QFile *file) {
    BoundingBox::writeBoundingBox(file);
    mPathEffectsAnimators->writePathEffectAnimators(file);
    mOutlinePathEffectsAnimators->writePathEffectAnimators(file);
    mFillGradientPoints->writeGradientPoints(file);
    mStrokeGradientPoints->writeGradientPoints(file);
    mFillSettings->writePaintSettings(file);
    mStrokeSettings->writeStrokeSettings(file);
}

void PathBox::readBoundingBox(QFile *file) {
    BoundingBox::readBoundingBox(file);
    mPathEffectsAnimators->readPathEffectAnimators(file);
    mOutlinePathEffectsAnimators->readPathEffectAnimators(file);
    mFillGradientPoints->readGradientPoints(file);
    mStrokeGradientPoints->readGradientPoints(file);
    mFillSettings->readPaintSettings(file);
    mStrokeSettings->readStrokeSettings(file);
}

void ParticleEmitter::writeParticleEmitter(QFile *file) {
    mColorAnimator->writeColorAnimator(file);
    mPos->writeQPointFAnimator(file);
    mWidth->writeQrealAnimator(file);
    mSrcVelInfl->writeQrealAnimator(file);
    mIniVelocity->writeQrealAnimator(file);
    mIniVelocityVar->writeQrealAnimator(file);
    mIniVelocityAngle->writeQrealAnimator(file);
    mIniVelocityAngleVar->writeQrealAnimator(file);
    mAcceleration->writeQPointFAnimator(file);
    mParticlesPerSecond->writeQrealAnimator(file);
    mParticlesFrameLifetime->writeQrealAnimator(file);
    mVelocityRandomVar->writeQrealAnimator(file);
    mVelocityRandomVarPeriod->writeQrealAnimator(file);
    mParticleSize->writeQrealAnimator(file);
    mParticleSizeVar->writeQrealAnimator(file);
    mParticleLength->writeQrealAnimator(file);
    mParticlesDecayFrames->writeQrealAnimator(file);
    mParticlesSizeDecay->writeQrealAnimator(file);
    mParticlesOpacityDecay->writeQrealAnimator(file);
}

void ParticleEmitter::readParticleEmitter(QFile *file) {
    mColorAnimator->readColorAnimator(file);
    mPos->readQPointFAnimator(file);
    mWidth->readQrealAnimator(file);
    mSrcVelInfl->readQrealAnimator(file);
    mIniVelocity->readQrealAnimator(file);
    mIniVelocityVar->readQrealAnimator(file);
    mIniVelocityAngle->readQrealAnimator(file);
    mIniVelocityAngleVar->readQrealAnimator(file);
    mAcceleration->readQPointFAnimator(file);
    mParticlesPerSecond->readQrealAnimator(file);
    mParticlesFrameLifetime->readQrealAnimator(file);
    mVelocityRandomVar->readQrealAnimator(file);
    mVelocityRandomVarPeriod->readQrealAnimator(file);
    mParticleSize->readQrealAnimator(file);
    mParticleSizeVar->readQrealAnimator(file);
    mParticleLength->readQrealAnimator(file);
    mParticlesDecayFrames->readQrealAnimator(file);
    mParticlesSizeDecay->readQrealAnimator(file);
    mParticlesOpacityDecay->readQrealAnimator(file);
}

void ParticleBox::writeBoundingBox(QFile *file) {
    int nEmitters = mEmitters.count();
    file->write((char*)&nEmitters, sizeof(int));
    foreach(ParticleEmitter *emitter, mEmitters) {
        emitter->writeParticleEmitter(file);
    }
}

void ParticleBox::readBoundingBox(QFile *file) {
    int nEmitters;
    file->read((char*)&nEmitters, sizeof(int));
    for(int i = 0; i < nEmitters; i++) {
        ParticleEmitter *emitter = new ParticleEmitter(this);
        emitter->readParticleEmitter(file);
        addEmitter(emitter);
    }
}

void ImageBox::writeBoundingBox(QFile *file) {
    BoundingBox::writeBoundingBox(file);
    writeQString(file, mImageFilePath);
}

void ImageBox::readBoundingBox(QFile *file) {
    BoundingBox::readBoundingBox(file);
    readQString(file, &mImageFilePath);
}

void Circle::writeBoundingBox(QFile *file) {
    PathBox::writeBoundingBox(file);
    mHorizontalRadiusPoint->writeQPointFAnimator(file);
    mVerticalRadiusPoint->writeQPointFAnimator(file);
}

void Circle::readBoundingBox(QFile *file) {
    PathBox::readBoundingBox(file);
    mHorizontalRadiusPoint->readQPointFAnimator(file);
    mVerticalRadiusPoint->readQPointFAnimator(file);
}

void Rectangle::writeBoundingBox(QFile *file) {
    PathBox::writeBoundingBox(file);
    mRadiusPoint.writeQPointFAnimator(file);
    mTopLeftPoint->writeQPointFAnimator(file);
    mBottomRightPoint->writeQPointFAnimator(file);
}

void Rectangle::readBoundingBox(QFile *file) {
    PathBox::readBoundingBox(file);
    mRadiusPoint.readQPointFAnimator(file);
    mTopLeftPoint->readQPointFAnimator(file);
    mBottomRightPoint->readQPointFAnimator(file);
}

void VideoBox::writeBoundingBox(QFile *file) {
    BoundingBox::writeBoundingBox(file);
    writeQString(file, mSrcFilePath);
}

void VideoBox::readBoundingBox(QFile *file) {
    BoundingBox::readBoundingBox(file);
    readQString(file, &mSrcFilePath);
}

void Tile::writeTile(QFile *file) {
    file->write((char*)mData, TILE_DIM*TILE_DIM*4*sizeof(uchar));
}

void Tile::readTile(QFile *file) {
    file->read((char*)mData, TILE_DIM*TILE_DIM*4*sizeof(uchar));
    updateDrawerFromDataArray();
}

void TilesData::writeTilesData(QFile *file) {
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->writeTile(file);
        }
    }
}

void TilesData::readTilesData(QFile *file) {
    for(int i = 0; i < mNTileCols; i++) {
        for(int j = 0; j < mNTileRows; j++) {
            mTiles[j][i]->readTile(file);
        }
    }
}

void Surface::writeSurface(QFile *file) {
    file->write((char*)&mWidth, sizeof(ushort));
    file->write((char*)&mHeight, sizeof(ushort));
    mCurrentTiles->writeTilesData(file);
}

void Surface::readSurface(QFile *file) {
    file->read((char*)&mWidth, sizeof(ushort));
    file->read((char*)&mHeight, sizeof(ushort));
    setSize(mWidth, mHeight);
    mCurrentTiles->readTilesData(file);
}

void SurfaceKey::writeSurfaceKey(QFile *file) {
    Key::writeKey(file);
    mTiles->writeTilesData(file);
}

void SurfaceKey::readSurfaceKey(QFile *file) {
    Key::readKey(file);
    mTiles->readTilesData(file);
}

void AnimatedSurface::writeAnimatedSurface(QFile *file) {
    file->write((char*)&mWidth, sizeof(ushort));
    file->write((char*)&mHeight, sizeof(ushort));
    int nKeys = anim_mKeys.count();
    file->write((char*)&nKeys, sizeof(int));
    if(nKeys == 0) {
        mCurrentTiles->writeTilesData(file);
    } else {
        foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
            ((SurfaceKey*)key.get())->writeSurfaceKey(file);
        }
    }
}

void AnimatedSurface::readAnimatedSurface(QFile *file) {
    file->read((char*)&mWidth, sizeof(ushort));
    file->read((char*)&mHeight, sizeof(ushort));
    int nKeys;
    file->read((char*)&nKeys, sizeof(int));
    if(nKeys == 0) {
        setSize(mWidth, mHeight);
        mCurrentTiles->readTilesData(file);
    } else {
        for(int i = 0; i < nKeys; i++) {
            SurfaceKey *key = new SurfaceKey(this);
            key->setTiles(new TilesData(mWidth,
                                        mHeight, true));
            key->readSurfaceKey(file);
            anim_appendKey(key);
        }
        setSize(mWidth, mHeight);
    }
}

void PaintBox::writeBoundingBox(QFile *file) {
    BoundingBox::writeBoundingBox(file);
    mTopLeftPoint->writeQPointFAnimator(file);
    mBottomRightPoint->writeQPointFAnimator(file);
    mMainHandler->writeAnimatedSurface(file);
}

void PaintBox::readBoundingBox(QFile *file) {
    BoundingBox::readBoundingBox(file);
    mTopLeftPoint->readQPointFAnimator(file);
    mBottomRightPoint->readQPointFAnimator(file);
    finishSizeSetup();
    mMainHandler->readAnimatedSurface(file);
}

void ImageSequenceBox::writeBoundingBox(QFile *file) {
    BoundingBox::writeBoundingBox(file);
    int nFrames = mListOfFrames.count();
    file->write((char*)&nFrames, sizeof(int));
    foreach(const QString &frame, mListOfFrames) {
        writeQString(file, frame);
    }
}

void ImageSequenceBox::readBoundingBox(QFile *file) {
    BoundingBox::readBoundingBox(file);
    int nFrames;
    file->read((char*)&nFrames, sizeof(int));
    for(int i = 0; i < nFrames; i++) {
        QString frame;
        readQString(file, &frame);
        mListOfFrames << frame;
    }
}

void TextBox::writeBoundingBox(QFile *file) {
    PathBox::writeBoundingBox(file);
    mText->writeQStringAnimator(file);
    file->write((char*)&mAlignment, sizeof(Qt::Alignment));
    qreal fontSize = mFont.pointSizeF();
    QString fontFamily = mFont.family();
    QString fontStyle = mFont.styleName();
    file->write((char*)&fontSize, sizeof(qreal));
    writeQString(file, fontFamily);
    writeQString(file, fontStyle);
}

void TextBox::readBoundingBox(QFile *file) {
    PathBox::readBoundingBox(file);
    mText->readQStringAnimator(file);
    file->read((char*)&mAlignment, sizeof(Qt::Alignment));
    qreal fontSize;
    QString fontFamily;
    QString fontStyle;
    file->read((char*)&fontSize, sizeof(qreal));
    readQString(file, &fontFamily);
    readQString(file, &fontStyle);
    mFont.setPointSizeF(fontSize);
    mFont.setFamily(fontFamily);
    mFont.setStyleName(fontStyle);
}

void BoxesGroup::writeBoundingBox(QFile *file) {
    BoundingBox::writeBoundingBox(file);
    int nChildBoxes = mChildBoxes.count();
    file->write((char*)&nChildBoxes, sizeof(int));
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->writeBoundingBox(file);
    }
}

void BoxesGroup::readBoundingBox(QFile *file) {
    BoundingBox::readBoundingBox(file);
    int nChildBoxes;
    file->read((char*)&nChildBoxes, sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        BoundingBox *box;
        BoundingBoxType boxType;
        file->read((char*)&boxType, sizeof(BoundingBoxType));
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

        box->readBoundingBox(file);
        box->setParent(this);
        addChild(box);
    }
}

void PathAnimator::writePathAnimator(QFile *file) {
    int nPaths = mSinglePaths.count();
    file->write((char*)&nPaths, sizeof(int));
    foreach(VectorPathAnimator *pathAnimator, mSinglePaths) {
        pathAnimator->writeVectorPathAnimator(file);
    }
}

void PathAnimator::readPathAnimator(QFile *file) {
    int nPaths;
    file->read((char*)&nPaths, sizeof(int));
    for(int i = 0; i < nPaths; i++) {
        VectorPathAnimator *pathAnimator = new VectorPathAnimator(this);
        pathAnimator->readVectorPathAnimator(file);
        addSinglePathAnimator(pathAnimator, false);
    }
}

void VectorPath::writeBoundingBox(QFile *file) {
    PathBox::writeBoundingBox(file);
    mPathAnimator->writePathAnimator(file);
}

void VectorPath::readBoundingBox(QFile *file) {
    PathBox::readBoundingBox(file);
    mPathAnimator->readPathAnimator(file);
}

void Canvas::writeBoundingBox(QFile *file) {
    BoxesGroup::writeBoundingBox(file);
    file->write((char*)&mWidth, sizeof(int));
    file->write((char*)&mHeight, sizeof(int));
    file->write((char*)&mFps, sizeof(qreal));
    file->write((char*)&mCanvasTransformMatrix, sizeof(QMatrix));
}

void Canvas::readBoundingBox(QFile *file) {
    file->read((char*)&mType, sizeof(BoundingBoxType));
    BoxesGroup::readBoundingBox(file);
    file->read((char*)&mWidth, sizeof(int));
    file->read((char*)&mHeight, sizeof(int));
    file->read((char*)&mFps, sizeof(qreal));
    file->read((char*)&mCanvasTransformMatrix, sizeof(QMatrix));
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
}

void GradientWidget::writeGradients(QFile *file) {
    int nGradients = mGradients.count();
    file->write((char*)&nGradients, sizeof(int));
    foreach(const QSharedPointer<Gradient> &gradient, mGradients) {
        gradient->writeGradient(file);
    }
}

void GradientWidget::readGradients(QFile *file) {
    int nGradients;
    file->read((char*)&nGradients, sizeof(int));
    for(int i = 0; i < nGradients; i++) {
        Gradient *gradient = new Gradient();
        gradient->readGradient(file);
        addGradientToList(gradient);
        MainWindow::getInstance()->addLoadedGradient(gradient);
    }
}

void CanvasWindow::writeCanvases(QFile *file) {
    int nCanvases = mCanvasList.count();
    file->write((char*)&nCanvases, sizeof(int));
    int currentCanvasId = -1;
    int boxLoadId = 0;
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
        boxLoadId = canvas->setBoxLoadId(boxLoadId);
        canvas->writeBoundingBox(file);
        if(canvas == mCurrentCanvas) {
            currentCanvasId = mCurrentCanvas->getLoadId();
        }
    }
    file->write((char*)&currentCanvasId, sizeof(int));
}

void CanvasWindow::readCanvases(QFile *file) {
    int nCanvases;
    file->read((char*)&nCanvases, sizeof(int));
    for(int i = 0; i < nCanvases; i++) {
        FillStrokeSettingsWidget *fillStrokeSettingsWidget =
                MainWindow::getInstance()->getFillStrokeSettings();
        Canvas *canvas = new Canvas(fillStrokeSettingsWidget, this);
        canvas->readBoundingBox(file);
        MainWindow::getInstance()->addCanvas(canvas);
    }
    int currentCanvasId;
    file->read((char*)&currentCanvasId, sizeof(int));
    setCurrentCanvas((Canvas*)BoundingBox::getLoadedBoxById(currentCanvasId));
}

void MainWindow::loadAVFile(const QString &path) {
    QFile file(path);
    if(file.open(QIODevice::ReadOnly) ) {
        FileFooter footer;
        footer.read(&file);
        if(footer.combatybilityMode() ==
                FileFooter::CompatybilityMode::Compatible) {
            GradientWidget *gradientWidget = mFillStrokeSettings->getGradientWidget();
            gradientWidget->readGradients(&file);
            mCanvasWindow->readCanvases(&file);

            clearLoadedGradientsList();
            gradientWidget->clearGradientsLoadIds();
            BoundingBox::clearLoadedBoxes();
        } else {
            QMessageBox::critical(this, tr("File Load Fail"),
                                  tr("The file you tried to load is incompatible,\n"
                                     "or damaged."),
                                  QMessageBox::Ok,
                                  QMessageBox::Ok);
        }

        file.close();
    }
}

void MainWindow::saveToFile(const QString &path) {
    disable();
    QFile file(path);
    if(file.exists()) {
        file.remove();
    }

    GradientWidget *gradientWidget = mFillStrokeSettings->getGradientWidget();
    if(file.open(QIODevice::WriteOnly) ) {
        gradientWidget->setGradientLoadIds();
        gradientWidget->writeGradients(&file);
        mCanvasWindow->writeCanvases(&file);

        clearLoadedGradientsList();
        gradientWidget->clearGradientsLoadIds();

        FileFooter footer;
        footer.write(&file);

        file.close();
    }

    BoundingBox::clearLoadedBoxes();

    enable();
}
