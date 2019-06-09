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
#include "Sound/soundcomposition.h"
#include "Animators/gpueffectanimators.h"

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
        if(!target->seek(pos)) RuntimeThrow("Failed to seek to FileFooter");

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

void ComboBoxProperty::readProperty(QIODevice * const src) {
    src->read(rcChar(&mCurrentValue), sizeof(int));
}

void Key::writeKey(QIODevice * const dst) {
    dst->write(rcConstChar(&mRelFrame), sizeof(int));
}

void Key::readKey(QIODevice * const src) {
    src->read(rcChar(&mRelFrame), sizeof(int));
}

void Animator::writeSelectedKeys(QIODevice * const dst) {
    const int nKeys = anim_mSelectedKeys.count();
    dst->write(rcConstChar(&nKeys), sizeof(int));
    for(const auto& key : anim_mSelectedKeys)
        key->writeKey(dst);
}

void QrealKey::writeKey(QIODevice * const dst) {
    Key::writeKey(dst);
    dst->write(rcConstChar(&mValue), sizeof(qreal));

    dst->write(rcConstChar(&mStartEnabled), sizeof(bool));
    dst->write(rcConstChar(&mStartPt), sizeof(ClampedPoint));

    dst->write(rcConstChar(&mEndEnabled), sizeof(bool));
    dst->write(rcConstChar(&mEndPt), sizeof(ClampedPoint));
}

void QrealKey::readKey(QIODevice * const src) {
    Key::readKey(src);
    src->read(rcChar(&mValue), sizeof(qreal));

    src->read(rcChar(&mStartEnabled), sizeof(bool));
    src->read(rcChar(&mStartPt), sizeof(ClampedPoint));

    src->read(rcChar(&mEndEnabled), sizeof(bool));
    src->read(rcChar(&mEndPt), sizeof(ClampedPoint));
}

void QrealAnimator::writeProperty(QIODevice * const dst) const {
    writeKeys(dst);
    dst->write(rcConstChar(&mCurrentBaseValue), sizeof(qreal));
}

stdsptr<Key> QrealAnimator::readKey(QIODevice * const src) {
    auto newKey = SPtrCreate(QrealKey)(this);
    newKey->readKey(src);
    return std::move(newKey);
}

void QrealAnimator::readProperty(QIODevice * const src) {
    readKeys(src);

    qreal val;
    src->read(rcChar(&val), sizeof(qreal));
    setCurrentBaseValue(val);
}

void ColorAnimator::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mColorMode), sizeof(ColorMode));
}

void ColorAnimator::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    src->read(rcChar(&mColorMode), sizeof(ColorMode));
    setColorMode(mColorMode);
}

void PixmapEffect::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mType), sizeof(PixmapEffectType));
}

void PixmapEffect::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mVisible), sizeof(bool));
}

void PixmapEffect::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    src->read(rcChar(&mVisible), sizeof(bool));
}

void Gradient::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mLoadId), sizeof(int));
    const int nColors = mColors.count();
    target->write(rcConstChar(&nColors), sizeof(int));
    for(const auto& color : mColors) {
        color->writeProperty(target);
    }
}

void Gradient::readProperty(QIODevice * const src) {
    src->read(rcChar(&mLoadId), sizeof(int));
    int nColors;
    src->read(rcChar(&nColors), sizeof(int));
    for(int i = 0; i < nColors; i++) {
        const auto colorAnim = SPtrCreate(ColorAnimator)();
        colorAnim->readProperty(src);
        addColorToList(colorAnim);
    }
    updateQGradientStops(UpdateReason::USER_CHANGE);
}

void BrushSettings::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    gWrite(dst, mBrush ? mBrush->getCollectionName() : "");
    gWrite(dst, mBrush ? mBrush->getBrushName() : "");
}

void BrushSettings::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    const QString brushCollection = gReadString(src);
    const QString brushName = gReadString(src);
    mBrush = BrushSelectionWidget::sGetBrush(brushCollection, brushName);
}

void OutlineSettingsAnimator::writeProperty(QIODevice * const dst) const {
    PaintSettingsAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mCapStyle), sizeof(Qt::PenCapStyle));
    dst->write(rcConstChar(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    dst->write(rcConstChar(&mOutlineCompositionMode),
               sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::readProperty(QIODevice * const src) {
    PaintSettingsAnimator::readProperty(src);
    src->read(rcChar(&mCapStyle), sizeof(Qt::PenCapStyle));
    src->read(rcChar(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    src->read(rcChar(&mOutlineCompositionMode),
              sizeof(QPainter::CompositionMode));
}

void PaintSettingsAnimator::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mPaintType), sizeof(PaintType));
    dst->write(rcConstChar(&mGradientType), sizeof(bool));
    const int gradId = mGradient ? mGradient->getLoadId() : -1;
    dst->write(rcConstChar(&gradId), sizeof(int));
}

void PaintSettingsAnimator::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    PaintType paintType;
    src->read(rcChar(&paintType), sizeof(PaintType));
    int gradId;
    src->read(rcChar(&mGradientType), sizeof(bool));
    src->read(rcChar(&gradId), sizeof(int));
    if(gradId != -1) {
        mGradient = MainWindow::getInstance()->getLoadedGradientById(gradId);
    }
    setPaintType(paintType);
}

void DurationRectangle::writeDurationRectangle(QIODevice *dst) {
    const int minFrame = getMinFrame();
    const int maxFrame = getMaxFrame();
    const int framePos = getFramePos();
    dst->write(rcConstChar(&minFrame), sizeof(int));
    dst->write(rcConstChar(&maxFrame), sizeof(int));
    dst->write(rcConstChar(&framePos), sizeof(int));
}

void DurationRectangle::readDurationRectangle(QIODevice *src) {
    int minFrame;
    int maxFrame;
    int framePos;
    src->read(rcChar(&minFrame), sizeof(int));
    src->read(rcChar(&maxFrame), sizeof(int));
    src->read(rcChar(&framePos), sizeof(int));
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
    StaticComplexAnimator::writeProperty(target);

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
}

void BoundingBox::readBoundingBox(QIODevice * const target) {
    StaticComplexAnimator::readProperty(target);
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

    if(hasDurRect) anim_shiftAllKeys(prp_getFrameShift());

    BoundingBox::sAddReadBox(this);
}

void PathEffect::writeProperty(QIODevice * const target) const {
    target->write(rcConstChar(&mVisible), sizeof(bool));
}

void PathEffect::readProperty(QIODevice * const src) {
    src->read(rcChar(&mVisible), sizeof(bool));
    bool tmp;
    src->read(rcChar(&tmp), sizeof(bool));
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

void BoxTargetProperty::readProperty(QIODevice * const src) {
    int targetReadId;
    src->read(rcChar(&targetReadId), sizeof(int));
    int targetDocumentId;
    src->read(rcChar(&targetDocumentId), sizeof(int));
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

#include "Animators/SmartPath/smartpathcollection.h"

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

void VideoBox::writeBoundingBox(QIODevice * const target) {
    AnimationBox::writeBoundingBox(target);
    gWrite(target, mSrcFilePath);
}

void VideoBox::readBoundingBox(QIODevice * const target) {
    AnimationBox::readBoundingBox(target);
    QString path;
    gRead(target, path);
    setFilePath(path);
}

void AnimationBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    target->write(rcConstChar(&mFrameRemappingEnabled), sizeof(bool));
}

void AnimationBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    bool frameRemapping;
    target->read(rcChar(&frameRemapping), sizeof(bool));
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
    QFile file(path);
    if(file.exists()) file.remove();

    if(file.open(QIODevice::WriteOnly)) {
        auto gradientWidget = mFillStrokeSettings->getGradientWidget();
        gradientWidget->setGradientLoadIds();
        gradientWidget->writeGradients(&file);
        mCanvasWindow->writeCanvases(&file);

        clearLoadedGradientsList();
        gradientWidget->clearGradientsLoadIds();

        FileFooter::sWrite(&file);

        file.close();
    } else {
        RuntimeThrow("Could not open file for writing " + path + ".");
    }

    BoundingBox::sClearWriteBoxes();
    addRecentFile(path);
}
