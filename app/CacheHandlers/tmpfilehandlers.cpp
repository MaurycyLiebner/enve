#include "tmpfilehandlers.h"
#include "imagecachecontainer.h"
#include "castmacros.h"

CacheContainerTmpFileDataLoader::CacheContainerTmpFileDataLoader(
        const qsptr<QTemporaryFile> &file,
        ImageCacheContainer *target) : mTargetCont(target) {
    mTmpFile = file;
}

void CacheContainerTmpFileDataLoader::_processUpdate() {
    if(mTmpFile->open()) {
        int width, height;
        mTmpFile->read(rcChar(&width), sizeof(int));
        mTmpFile->read(rcChar(&height), sizeof(int));
        SkBitmap btmp;
        const auto info = SkiaHelpers::getPremulBGRAInfo(width, height);
        btmp.allocPixels(info);
        const qint64 readBytes = width*height*4*
                static_cast<qint64>(sizeof(uchar));
        mTmpFile->read(scChar(btmp.getPixels()), readBytes);
        mImage = SkImage::MakeFromBitmap(btmp);

        mTmpFile->close();
    }
}

void CacheContainerTmpFileDataLoader::afterProcessingFinished() {
    mTargetCont->setDataLoadedFromTmpFile(mImage);
    _ScheduledTask::afterProcessingFinished();
}

CacheContainerTmpFileDataSaver::CacheContainerTmpFileDataSaver(
        const sk_sp<SkImage> &image,
        ImageCacheContainer *target) : mTargetCont(target) {
    mImage = image;
}

void CacheContainerTmpFileDataSaver::_processUpdate() {
    // mSavingFailed = true; return; // NO TMP FILES !!!
    SkPixmap pix;
    if(!mImage->peekPixels(&pix)) {
        if(!mImage->makeRasterImage()->peekPixels(&pix)) {
            mSavingFailed = true;
            return;
        }
    }
    mTmpFile = qsptr<QTemporaryFile>(new QTemporaryFile());
    if(mTmpFile->open()) {
        const int width = pix.width();
        const int height = pix.height();
        mTmpFile->write(rcConstChar(&width), sizeof(int));
        mTmpFile->write(rcConstChar(&height), sizeof(int));
        const qint64 writeBytes = width*height*4*
                static_cast<qint64>(sizeof(uchar));
        mTmpFile->write(rcConstChar(pix.writable_addr()), writeBytes);
        mTmpFile->close();
    } else {
        mSavingFailed = true;
    }
}

void CacheContainerTmpFileDataSaver::afterProcessingFinished() {
    if(mSavingFailed) {
        if(!mTargetCont->freeAndRemove()) {

        }
    } else {
        mTargetCont->setDataSavedToTmpFile(mTmpFile);
    }
    _ScheduledTask::afterProcessingFinished();
}

void CacheContainerTmpFileDataDeleter::_processUpdate() {
    mTmpFile.reset();
}
