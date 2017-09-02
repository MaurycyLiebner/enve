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
#include "canvas.h"
#include "durationrectangle.h"
#include "gradientpoints.h"
#include "gradientpoint.h"
#include "qrealkey.h"
#include "mainwindow.h"
#include "Colors/ColorWidgets/gradientwidget.h"
// each returns written size

#define FORMAT_STR "AniVect av"
#define CREATOR_VERSION "0.1a"
#define CREATOR_APPLICATION "AniVect"

void writeQString(std::fstream *file,
                 const QString &strToWrite) {
    int nChars = strToWrite.length();
    file->write((char*)&nChars, sizeof(int));
    file->write((char*)strToWrite.utf16(), nChars*sizeof(ushort));
}

void readQString(std::fstream *file,
                QString *targetStr) {
    int nChars;
    file->read((char*)&nChars, sizeof(int));
    ushort *chars = new ushort[nChars];

    file->read((char*)chars, nChars*sizeof(ushort));
    *targetStr = QString::fromUtf16(chars, nChars);
    delete[] chars;
}

struct FileFooter {
    char formatStr[15] = FORMAT_STR;
    char creatorVersion[15] = CREATOR_VERSION;
    char creatorApplication[15] = CREATOR_APPLICATION;

    void write(std::fstream *file) {
        file->write((char*)this, sizeof(FileFooter));
    }

    void read(std::fstream *file) {
        file->seekg(-sizeof(FileFooter), std::ios_base::end);
        file->read((char*)this, sizeof(FileFooter));
        file->seekg(0);
    }
};

void BoolProperty::writeBoolProperty(std::fstream *file) {
    file->write((char*)&mValue, sizeof(bool));
}

void BoolProperty::readBoolProperty(std::fstream *file) {
    file->read((char*)&mValue, sizeof(bool));
}

void Key::writeKey(std::fstream *file) {
    file->write((char*)&mRelFrame, sizeof(int));
}

void Key::readKey(std::fstream *file) {
    file->read((char*)&mRelFrame, sizeof(int));
}

void PathContainer::writePathContainer(std::fstream *file) {
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

void PathContainer::readPathContainer(std::fstream *file) {
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

void PathKey::writePathKey(std::fstream *file) {
    writeKey(file);
    writePathContainer(file);
}

void PathKey::readPathKey(std::fstream *file) {
    readKey(file);
    readPathContainer(file);
}

void VectorPathAnimator::writeVectorPathAnimator(std::fstream *file) {
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

void VectorPathAnimator::readVectorPathAnimator(std::fstream *file) {
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

void QrealKey::writeQrealKey(std::fstream *file) {
    writeKey(file);
    file->write((char*)&mValue, sizeof(qreal));

    file->write((char*)&mStartEnabled, sizeof(bool));
    file->write((char*)&mStartFrame, sizeof(qreal));
    file->write((char*)&mStartValue, sizeof(qreal));

    file->write((char*)&mEndEnabled, sizeof(bool));
    file->write((char*)&mEndFrame, sizeof(qreal));
    file->write((char*)&mEndValue, sizeof(qreal));
}

void QrealKey::readQrealKey(std::fstream *file) {
    readKey(file);
    file->read((char*)&mValue, sizeof(qreal));

    file->read((char*)&mStartEnabled, sizeof(bool));
    file->read((char*)&mStartFrame, sizeof(qreal));
    file->read((char*)&mStartValue, sizeof(qreal));

    file->read((char*)&mEndEnabled, sizeof(bool));
    file->read((char*)&mEndFrame, sizeof(qreal));
    file->read((char*)&mEndValue, sizeof(qreal));
}

void QrealAnimator::writeQrealAnimator(std::fstream *file) {
    int nKeys = anim_mKeys.count();
    file->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QrealKey*)key.get())->writeQrealKey(file);
    }

    file->write((char*)&mCurrentValue, sizeof(qreal));
}

void QrealAnimator::readQrealAnimator(std::fstream *file) {
    int nKeys;
    file->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        QrealKey *newKey = new QrealKey(this);
        newKey->readQrealKey(file);
        anim_appendKey(newKey, false, false);
    }

    file->read((char*)&mCurrentValue, sizeof(qreal));
}

void QPointFAnimator::writeQPointFAnimator(std::fstream *file) {
    mXAnimator->writeQrealAnimator(file);
    mYAnimator->writeQrealAnimator(file);
}

void QPointFAnimator::readQPointFAnimator(std::fstream *file) {
    mXAnimator->readQrealAnimator(file);
    mYAnimator->readQrealAnimator(file);
}

void ColorAnimator::writeColorAnimator(std::fstream *file) {
    file->write((char*)&mColorMode, sizeof(ColorMode));
    mVal1Animator->writeQrealAnimator(file);
    mVal2Animator->writeQrealAnimator(file);
    mVal3Animator->writeQrealAnimator(file);
    mAlphaAnimator->writeQrealAnimator(file);
}

void ColorAnimator::readColorAnimator(std::fstream *file) {
    file->read((char*)&mColorMode, sizeof(ColorMode));
    setColorMode(mColorMode);
    mVal1Animator->readQrealAnimator(file);
    mVal2Animator->readQrealAnimator(file);
    mVal3Animator->readQrealAnimator(file);
    mAlphaAnimator->readQrealAnimator(file);
}

void QStringKey::writeQStringKey(std::fstream *file) {
    writeQString(file, mText);
}

void QStringKey::readQStringKey(std::fstream *file) {
    readQString(file, &mText);
}

void QStringAnimator::writeQStringAnimator(std::fstream *file) {
    int nKeys = anim_mKeys.count();
    file->write((char*)&nKeys, sizeof(int));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QStringKey*)key.get())->writeQStringKey(file);
    }
    writeQString(file, mCurrentText);
}

void QStringAnimator::readQStringAnimator(std::fstream *file) {
    int nKeys;
    file->read((char*)&nKeys, sizeof(int));
    for(int i = 0; i < nKeys; i++) {
        QStringKey *newKey = new QStringKey("", 0, this);
        newKey->readQStringKey(file);
        anim_appendKey(newKey, false, false);
    }
    readQString(file, &mCurrentText);
}

void PixmapEffect::writePixmapEffect(std::fstream *file) {
    file->write((char*)&mType, sizeof(PixmapEffectType));
}

void BlurEffect::readBlurEffect(std::fstream *file) {
    mHighQuality->readBoolProperty(file);
    mBlurRadius->readQrealAnimator(file);
}

void BlurEffect::writePixmapEffect(std::fstream *file) {
    PixmapEffect::writePixmapEffect(file);
    mHighQuality->writeBoolProperty(file);
    mBlurRadius->writeQrealAnimator(file);
}

void EffectAnimators::writeEffectAnimators(std::fstream *file) {
    int nEffects = ca_mChildAnimators.count();
    file->write((char*)&nEffects, sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PixmapEffect*)effect.data())->writePixmapEffect(file);
    }
}

void EffectAnimators::readEffectAnimators(std::fstream *file) {
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

void BasicTransformAnimator::writeBasicTransformAnimator(std::fstream *file) {
    mPosAnimator->writeQPointFAnimator(file);
    mScaleAnimator->writeQPointFAnimator(file);
    mRotAnimator->writeQrealAnimator(file);
}

void BasicTransformAnimator::readBasicTransformAnimator(std::fstream *file) {
    mPosAnimator->readQPointFAnimator(file);
    mScaleAnimator->readQPointFAnimator(file);
    mRotAnimator->readQrealAnimator(file);
}

void BoxTransformAnimator::writeBoxTransformAnimator(std::fstream *file) {
    writeBasicTransformAnimator(file);
    mOpacityAnimator->writeQrealAnimator(file);
    mPivotAnimator->writeQPointFAnimator(file);
}

void BoxTransformAnimator::readBoxTransformAnimator(std::fstream *file) {
    readBasicTransformAnimator(file);
    mOpacityAnimator->readQrealAnimator(file);
    mPivotAnimator->readQPointFAnimator(file);
}

void GradientPoints::writeGradientPoints(std::fstream *file) {
    startPoint->writeQPointFAnimator(file);
    endPoint->writeQPointFAnimator(file);
}

void GradientPoints::readGradientPoints(std::fstream *file) {
    startPoint->readQPointFAnimator(file);
    endPoint->readQPointFAnimator(file);
}

void Gradient::writeGradient(std::fstream *file) {
    file->write((char*)&mLoadId, sizeof(int));
    int nColors = mColors.count();
    file->write((char*)&nColors, sizeof(int));
    foreach(ColorAnimator *color, mColors) {
        color->writeColorAnimator(file);
    }
}

void Gradient::readGradient(std::fstream *file) {
    file->read((char*)&mLoadId, sizeof(int));
    int nColors;
    file->read((char*)&nColors, sizeof(int));
    for(int i = 0; i < nColors; i++) {
        ColorAnimator *colorAnim = new ColorAnimator();
        colorAnim->readColorAnimator(file);
        addColorToList(colorAnim, false);
    }
}

void StrokeSettings::writeStrokeSettings(std::fstream *file) {
    writePaintSettings(file);
    mLineWidth->writeQrealAnimator(file);
    file->write((char*)&mCapStyle, sizeof(Qt::PenCapStyle));
    file->write((char*)&mJoinStyle, sizeof(Qt::PenJoinStyle));
    file->write((char*)&mOutlineCompositionMode,
               sizeof(QPainter::CompositionMode));
}

void StrokeSettings::readStrokeSettings(std::fstream *file) {
    readPaintSettings(file);
    mLineWidth->readQrealAnimator(file);
    file->read((char*)&mCapStyle, sizeof(Qt::PenCapStyle));
    file->read((char*)&mJoinStyle, sizeof(Qt::PenJoinStyle));
    file->read((char*)&mOutlineCompositionMode,
                sizeof(QPainter::CompositionMode));
}

void PaintSettings::writePaintSettings(std::fstream *file) {
    mGradientPoints->writeGradientPoints(file);
    mColor->writeColorAnimator(file);
    file->write((char*)&mPaintType, sizeof(PaintType));
    int gradId;
    if(mGradient == NULL) {
        gradId = -1;
    } else {
        gradId = mGradient->getLoadId();
    }
    file->write((char*)&gradId, sizeof(int));
}

void PaintSettings::readPaintSettings(std::fstream *file) {
    mGradientPoints->readGradientPoints(file);
    mColor->readColorAnimator(file);
    file->read((char*)&mPaintType, sizeof(PaintType));
    int gradId;
    file->read((char*)&gradId, sizeof(int));
    if(gradId != -1) {
        mGradient = MainWindow::getInstance()->
                getLoadedGradientById(gradId)->ref<Gradient>();
    }
}

void DurationRectangle::writeDurationRectangle(std::fstream *file) {
    int minFrame = getMinFrame();
    int maxFrame = getMaxFrame();
    int framePos = getFramePos();
    file->write((char*)&minFrame, sizeof(int));
    file->write((char*)&maxFrame, sizeof(int));
    file->write((char*)&framePos, sizeof(int));
}

void DurationRectangle::readDurationRectangle(std::fstream *file) {
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

void BoundingBox::writeBoundingBox(std::fstream *file) {
    file->write((char*)&mType, sizeof(BoundingBoxType));
    writeQString(file, prp_mName);
    file->write((char*)&mLoadId, sizeof(int));
    file->write((char*)&mPivotChanged, sizeof(bool));
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

void BoundingBox::readBoundingBox(std::fstream *file) {
    readQString(file, &prp_mName);
    file->read((char*)&mLoadId, sizeof(int));
    file->read((char*)&mPivotChanged, sizeof(bool));
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
}

void PathEffect::writePathEffect(std::fstream *file) {
    file->write((char*)&mPathEffectType, sizeof(PathEffectType));
}

void DisplacePathEffect::writePathEffect(std::fstream *file) {
    PathEffect::writePathEffect(file);
    mSegLength->writeQrealAnimator(file);
    mMaxDev->writeQrealAnimator(file);
    mSmoothness->writeQrealAnimator(file);
}

void DisplacePathEffect::readDisplacePathEffect(std::fstream *file) {
    mSegLength->readQrealAnimator(file);
    mMaxDev->readQrealAnimator(file);
    mSmoothness->readQrealAnimator(file);
}

void PathEffectAnimators::writePathEffectAnimators(std::fstream *file) {
    int nEffects = ca_mChildAnimators.count();
    file->write((char*)&nEffects, sizeof(int));
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        ((PathEffect*)effect.data())->writePathEffect(file);
    }
}

void PathEffectAnimators::readPathEffectAnimators(std::fstream *file) {
    int nEffects;
    file->read((char*)&nEffects, sizeof(int));
    for(int i = 0; i < nEffects; i++) {
        PathEffectType typeT;
        file->read((char*)&typeT, sizeof(PathEffectType));
        if(typeT == DISPLACE_PATH_EFFECT) {
            DisplacePathEffect *displaceEffect = new DisplacePathEffect();
            displaceEffect->readDisplacePathEffect(file);
            addEffect(displaceEffect);
        }
    }
}

void PathBox::writeBoundingBox(std::fstream *file) {
    BoundingBox::writeBoundingBox(file);
    mPathEffectsAnimators->writePathEffectAnimators(file);
    mOutlinePathEffectsAnimators->writePathEffectAnimators(file);
    mFillGradientPoints->writeGradientPoints(file);
    mStrokeGradientPoints->writeGradientPoints(file);
    mFillSettings->writePaintSettings(file);
    mStrokeSettings->writeStrokeSettings(file);
}

void PathBox::readBoundingBox(std::fstream *file) {
    BoundingBox::readBoundingBox(file);
    mPathEffectsAnimators->readPathEffectAnimators(file);
    mOutlinePathEffectsAnimators->readPathEffectAnimators(file);
    mFillGradientPoints->readGradientPoints(file);
    mStrokeGradientPoints->readGradientPoints(file);
    mFillSettings->readPaintSettings(file);
    mStrokeSettings->readStrokeSettings(file);
}

void BoxesGroup::writeBoundingBox(std::fstream *file) {
    BoundingBox::writeBoundingBox(file);
    int nChildBoxes = mChildBoxes.count();
    file->write((char*)&nChildBoxes, sizeof(int));
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->writeBoundingBox(file);
    }
}

void BoxesGroup::readBoundingBox(std::fstream *file) {
    BoundingBox::readBoundingBox(file);
    int nChildBoxes;
    file->read((char*)&nChildBoxes, sizeof(int));
    for(int i = 0; i < nChildBoxes; i++) {
        BoundingBox *box;
        BoundingBoxType boxType;
        file->read((char*)&boxType, sizeof(BoundingBoxType));
        if(boxType == TYPE_VECTOR_PATH) {
            box = new VectorPath();
        }

        box->readBoundingBox(file);
        addChild(box);
    }
}

void PathAnimator::writePathAnimator(std::fstream *file) {
    int nPaths = mSinglePaths.count();
    file->write((char*)&nPaths, sizeof(int));
    foreach(VectorPathAnimator *pathAnimator, mSinglePaths) {
        pathAnimator->writeVectorPathAnimator(file);
    }
}

void PathAnimator::readPathAnimator(std::fstream *file) {
    int nPaths;
    file->read((char*)&nPaths, sizeof(int));
    for(int i = 0; i < nPaths; i++) {
        VectorPathAnimator *pathAnimator = new VectorPathAnimator(this);
        pathAnimator->readVectorPathAnimator(file);
        addSinglePathAnimator(pathAnimator, false);
    }
}

void VectorPath::writeBoundingBox(std::fstream *file) {
    PathBox::writeBoundingBox(file);
    mPathAnimator->writePathAnimator(file);
}

void VectorPath::readBoundingBox(std::fstream *file) {
    PathBox::readBoundingBox(file);
    mPathAnimator->readPathAnimator(file);
}

void Canvas::writeBoundingBox(std::fstream *file) {
    BoxesGroup::writeBoundingBox(file);
    file->write((char*)&mWidth, sizeof(int));
    file->write((char*)&mHeight, sizeof(int));
    file->write((char*)&mFps, sizeof(qreal));
}

void Canvas::readBoundingBox(std::fstream *file) {
    file->read((char*)&mType, sizeof(BoundingBoxType));
    BoxesGroup::readBoundingBox(file);
    file->read((char*)&mWidth, sizeof(int));
    file->read((char*)&mHeight, sizeof(int));
    file->read((char*)&mFps, sizeof(qreal));
}

void GradientWidget::writeGradients(std::fstream *file) {
    int nGradients = mGradients.count();
    file->write((char*)&nGradients, sizeof(int));
    foreach(const QSharedPointer<Gradient> &gradient, mGradients) {
        gradient->writeGradient(file);
    }
}

void GradientWidget::readGradients(std::fstream *file) {
    int nGradients;
    file->read((char*)&nGradients, sizeof(int));
    for(int i = 0; i < nGradients; i++) {
        Gradient *gradient = new Gradient();
        gradient->readGradient(file);
        addGradientToList(gradient);
        MainWindow::getInstance()->addLoadedGradient(gradient);
    }
}

void CanvasWindow::writeCanvases(std::fstream *file) {
    int nCanvases = mCanvasList.count();
    file->write((char*)&nCanvases, sizeof(int));
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
        canvas->writeBoundingBox(file);
    }
}

void CanvasWindow::readCanvases(std::fstream *file) {
    int nCanvases;
    file->read((char*)&nCanvases, sizeof(int));
    for(int i = 0; i < nCanvases; i++) {
        FillStrokeSettingsWidget *fillStrokeSettingsWidget =
                MainWindow::getInstance()->getFillStrokeSettings();
        Canvas *canvas = new Canvas(fillStrokeSettingsWidget, this);
        canvas->readBoundingBox(file);
        MainWindow::getInstance()->addCanvas(canvas);
    }
}
