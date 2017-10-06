#include "boundingboxrendercontainer.h"
#include "boundingbox.h"
#include <QElapsedTimer>
#include "memoryhandler.h"
#include "rendercachehandler.h"
#include "mainwindow.h"

RenderContainer::~RenderContainer() {

}

void RenderContainer::drawSk(SkCanvas *canvas, SkPaint *paint) {
    if(mSrcRenderData == NULL) return;
    canvas->save();
    canvas->concat(QMatrixToSkMatrix(mPaintTransform));
    if(paint != NULL) {
        if(paint->getBlendMode() == SkBlendMode::kDstIn ||
           paint->getBlendMode() == SkBlendMode::kSrcIn ||
           paint->getBlendMode() == SkBlendMode::kDstATop) {
            SkPaint paintT;
            paintT.setBlendMode(paint->getBlendMode());
            paintT.setColor(SK_ColorTRANSPARENT);
            SkPath path;
            path.addRect(SkRect::MakeXYWH(mDrawPos.x(), mDrawPos.y(),
                                          mImageSk->width(),
                                          mImageSk->height()));
            path.toggleInverseFillType();
            canvas->drawPath(path, paintT);
        }
    }
    canvas->drawImage(mImageSk, mDrawPos.x(), mDrawPos.y(), paint);
    canvas->restore();
}

void RenderContainer::updatePaintTransformGivenNewCombinedTransform(
                                    const QMatrix &combinedTransform) {
    mPaintTransform = mTransform.inverted()*combinedTransform;
    mPaintTransform.scale(1./mResolutionFraction,
                          1./mResolutionFraction);
}

void RenderContainer::setTransform(const QMatrix &transform) {
    mTransform = transform;
}

const QMatrix &RenderContainer::getTransform() const {
    return mTransform;
}

const QMatrix &RenderContainer::getPaintTransform() const {
    return mPaintTransform;
}

const SkPoint &RenderContainer::getDrawpos() const {
    return mDrawPos;
}

const qreal &RenderContainer::getResolutionFraction() const {
    return mResolutionFraction;
}

void RenderContainer::setVariablesFromRenderData(BoundingBoxRenderData *data) {
    mNoDataInMemory = false;
    mTmpFile.reset();

    mTransform = data->transform;
    mResolutionFraction = data->resolution;
    mImageSk = data->renderedImage;
    mDrawPos = data->drawPos;
    mRelFrame = data->relFrame;
    mPaintTransform.reset();
    mPaintTransform.scale(1./mResolutionFraction,
                          1./mResolutionFraction);
    mSrcRenderData = data->ref<BoundingBoxRenderData>();
    MemoryHandler::getInstance()->containerUpdated(this);
}

MinimalCacheContainer::MinimalCacheContainer(const bool &addToMemoryHandler) {
    if(addToMemoryHandler) {
        MemoryHandler::getInstance()->addContainer(this);
    }
}

MinimalCacheContainer::~MinimalCacheContainer() {
    //MemoryChecker::getInstance()->decUsedMemory(mImage.byteCount());
    if(MemoryHandler::getInstance() == NULL) return;
    MemoryHandler::getInstance()->removeContainer(this);
}

bool MinimalCacheContainer::freeAndRemoveFromMemoryHandler() {
    if(freeThis()) {
        MemoryHandler::getInstance()->removeContainer(this);
        return true;
    }
    return false;
}

CacheContainer::~CacheContainer() {
}

void CacheContainer::setParentCacheHandler(CacheHandler *handler) {
    mParentCacheHandler = handler;
}

void CacheContainer::replaceImageSk(const sk_sp<SkImage> &img) {
    mImageSk = img;
    if(mNoDataInMemory) {
        mNoDataInMemory = false;
        mTmpFile.reset();
    }
}

bool CacheContainer::freeThis() {
    if(mBlocked || mNoDataInMemory) return false;
    if(mParentCacheHandler == NULL) return false;
    MemoryHandler::getInstance()->incMemoryAwaitingRelease(getByteCount());
    saveToTmpFile();
    return true;
}

void CacheContainer::setBlocked(const bool &bT) {
    if(bT == mBlocked) return;
    mBlocked = bT;
    if(bT) {
        MemoryHandler::getInstance()->removeContainer(this);
        scheduleLoadFromTmpFile();
    } else {
        MemoryHandler::getInstance()->addContainer(this);
    }
}

void CacheContainer::setDataLoadedFromTmpFile(const sk_sp<SkImage> &img) {
    mLoadingFromFile = false;
    mNoDataInMemory = false;
    mImageSk = img;
    if(!mBlocked) {
        MemoryHandler::getInstance()->addContainer(this);
    }
}

const int &CacheContainer::getMinRelFrame() const {
    return mMinRelFrame;
}

const int &CacheContainer::getMaxRelFrame() const {
    return mMaxRelFrame;
}

bool CacheContainer::relFrameInRange(const int &relFrame) {
    return relFrame >= mMinRelFrame && relFrame < mMaxRelFrame;
}

void CacheContainer::setRelFrame(const int &frame) {
    mMinRelFrame = frame;
    mMaxRelFrame = frame + 1;
}

void CacheContainer::setMaxRelFrame(const int &maxFrame) {
    mMaxRelFrame = maxFrame;
}

void CacheContainer::setMinRelFrame(const int &minFrame) {
    mMinRelFrame = minFrame;
}

void CacheContainer::setRelFrameRange(const int &minFrame,
                                      const int &maxFrame) {
    mMinRelFrame = minFrame;
    mMaxRelFrame = maxFrame;
}

void CacheContainer::drawSk(SkCanvas *canvas) {
    canvas->drawImage(mImageSk, 0, 0);
}

void CacheContainer::setDataSavedToTmpFile(
        const QSharedPointer<QTemporaryFile> &tmpFile) {
    mSavingUpdatable = NULL;
    mTmpFile = tmpFile;
    mSavingToFile = false;
    if(mCancelAfterSaveDataClear) {
        mNoDataInMemory = false;
        mCancelAfterSaveDataClear = false;
        if(!mBlocked) {
            MemoryHandler::getInstance()->addContainer(this);
        }
        return;
    } else {
        mNoDataInMemory = true;
    }
    MemoryHandler::getInstance()->incMemoryAwaitingRelease(-getByteCount());
    mImageSk.reset();
}

void CacheContainer::saveToTmpFile() {
    if(mSavingToFile || mLoadingFromFile) return;
    MemoryHandler::getInstance()->removeContainer(this);
    mSavingToFile = true;
    mNoDataInMemory = true;
    mCancelAfterSaveDataClear = false;
    mSavingUpdatable = new CacheContainerTmpFileDataSaver(mImageSk,
                                                          this);
    mSavingUpdatable->addScheduler();
}

CacheContainerTmpFileDataLoader::CacheContainerTmpFileDataLoader(
        const QSharedPointer<QTemporaryFile> &file,
        CacheContainer *target) {
    mTargetCont = target;
    mTmpFile = file;
}

void CacheContainerTmpFileDataLoader::processUpdate() {
    if(mTmpFile->open()) {
        int width, height;
        mTmpFile->read((char*)&width, sizeof(int));
        mTmpFile->read((char*)&height, sizeof(int));
        SkBitmap btmp;
        SkImageInfo info = SkImageInfo::Make(width,
                                             height,
                                             kBGRA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             nullptr);
        btmp.allocPixels(info);
        mTmpFile->read((char*)btmp.getPixels(),
                       width*height*4*sizeof(uchar));
        mImage = SkImage::MakeFromBitmap(btmp);

        mTmpFile->close();
    }
}

void CacheContainerTmpFileDataLoader::afterUpdate() {
    mTargetCont->setDataLoadedFromTmpFile(mImage);
    Updatable::afterUpdate();
}

void CacheContainerTmpFileDataLoader::addSchedulerNow() {
    MainWindow::getInstance()->addFileUpdateScheduler(this);
}

CacheContainerTmpFileDataSaver::CacheContainerTmpFileDataSaver(
        const sk_sp<SkImage> &image,
        CacheContainer *target) {
    mTargetCont = target;
    mImage = image;
}

void CacheContainerTmpFileDataSaver::processUpdate() {
    SkPixmap pix;
    mImage->peekPixels(&pix);
    mTmpFile = QSharedPointer<QTemporaryFile>(new QTemporaryFile());
    if(mTmpFile->open()) {
        int width = pix.width();
        int height = pix.height();
        mTmpFile->write((char*)&width, sizeof(int));
        mTmpFile->write((char*)&height, sizeof(int));
        mTmpFile->write((char*)pix.writable_addr(),
                        width*height*4*sizeof(uchar));
        mTmpFile->close();
    }
}

void CacheContainerTmpFileDataSaver::afterUpdate() {
    mTargetCont->setDataSavedToTmpFile(mTmpFile);
    Updatable::afterUpdate();
}

void CacheContainerTmpFileDataSaver::addSchedulerNow() {
    MainWindow::getInstance()->addFileUpdateScheduler(this);
}
