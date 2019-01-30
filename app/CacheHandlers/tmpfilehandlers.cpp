#include "tmpfilehandlers.h"
#include "imagecachecontainer.h"

CacheContainerTmpFileDataLoader::CacheContainerTmpFileDataLoader(
        const qsptr<QTemporaryFile> &file,
        ImageCacheContainer *target) : mTargetCont(target) {
    mTmpFile = file;
}

void CacheContainerTmpFileDataLoader::_processUpdate() {
    if(mTmpFile->open()) {
        int width, height;
        mTmpFile->read(reinterpret_cast<char*>(&width), sizeof(int));
        mTmpFile->read(reinterpret_cast<char*>(&height), sizeof(int));
        SkBitmap btmp;
        SkImageInfo info = SkImageInfo::Make(width,
                                             height,
                                             kBGRA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             nullptr);
        btmp.allocPixels(info);
        mTmpFile->read(static_cast<char*>(btmp.getPixels()),
                       width*height*4*static_cast<qint64>(sizeof(uchar)));
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
        int width = pix.width();
        int height = pix.height();
        mTmpFile->write(reinterpret_cast<char*>(&width), sizeof(int));
        mTmpFile->write(reinterpret_cast<char*>(&height), sizeof(int));
        mTmpFile->write(static_cast<char*>(pix.writable_addr()),
                        width*height*4*static_cast<qint64>(sizeof(uchar)));
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
