// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "videobox.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <QDebug>
#include "Sound/singlesound.h"
#include "canvas.h"
#include "Sound/soundcomposition.h"
#include "filesourcescache.h"
#include "fileshandler.h"

VideoBox::VideoBox() : AnimationBox(eBoxType::video) {
    prp_setName("Video");
    const auto flar = mDurationRectangle->ref<FixedLenAnimationRect>();
    mSound = enve::make_shared<SingleSound>(flar);
    ca_addChild(mSound);
    mSound->hide();
    mSound->SWT_hide();

    connect(this, &eBoxOrSound::parentChanged,
            mSound.get(), &eBoxOrSound::setParentGroup);
}

void VideoBox::writeBoundingBox(eWriteStream& dst) {
    AnimationBox::writeBoundingBox(dst);
    dst << mFileHandler->path();
}

void VideoBox::readBoundingBox(eReadStream& src) {
    AnimationBox::readBoundingBox(src);
    QString path; src >> path;
    setFilePath(path);
}

#include "GUI/edialogs.h"
void VideoBox::changeSourceFile() {
    const QString path = eDialogs::openFile(
                "Change Source", getFilePath(),
                "Video Files (*.mp4 *.mov *.avi *.mkv *.m4v)");
    if(!path.isEmpty()) setFilePath(path);
}

void VideoBox::setStretch(const qreal stretch) {
    AnimationBox::setStretch(stretch);
    mSound->setStretch(stretch);
}

void VideoBox::setFilePath(const QString &path) {
    if(mSrcFramesCache) {
        const auto videoSrc = static_cast<VideoFrameHandler*>(mSrcFramesCache.get());
        const auto oldDataHandler = videoSrc->getDataHandler();
        disconnect(mFileHandler, &VideoFileHandler::pathChanged,
                   this, &VideoBox::animationDataChanged);
        disconnect(mFileHandler, &VideoFileHandler::reloaded,
                   this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        disconnect(oldDataHandler, &VideoDataHandler::frameCountUpdated,
                   this, &VideoBox::updateDurationRectangleAnimationRange);
    }
    mSrcFramesCache.reset();

    mFileHandler = FilesHandler::sInstance->getFileHandler<VideoFileHandler>(path);
    const auto newDataHandler = mFileHandler->getFrameHandler();
    if(newDataHandler) {
        mSrcFramesCache = enve::make_shared<VideoFrameHandler>(newDataHandler);
        getAnimationDurationRect()->setRasterCacheHandler(
                    &newDataHandler->getCacheHandler());
        connect(mFileHandler, &VideoFileHandler::pathChanged,
                this, &VideoBox::animationDataChanged);
        connect(mFileHandler, &VideoFileHandler::reloaded,
                this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        connect(newDataHandler, &VideoDataHandler::frameCountUpdated,
                this, &VideoBox::updateDurationRectangleAnimationRange);
    } else {
        getAnimationDurationRect()->setRasterCacheHandler(nullptr);
    }

    animationDataChanged();
}

QString VideoBox::getFilePath() {
    if(mFileHandler) return mFileHandler->path();
    return "";
}

void VideoBox::animationDataChanged() {
    soundDataChanged();
    AnimationBox::animationDataChanged();
}

void VideoBox::soundDataChanged() {
    const auto pScene = getParentScene();
    const auto soundHandler = mFileHandler->getSoundHandler();
    if(soundHandler) {
        if(!mSound->SWT_isVisible()) {
            if(pScene) {
                pScene->getSoundComposition()->addSound(mSound);
            }
        }
        mDurationRectangle->setSoundCacheHandler(&soundHandler->getCacheHandler());
    } else {
        if(mSound->SWT_isVisible()) {
            if(pScene) {
                pScene->getSoundComposition()->removeSound(mSound);
            }
        }
        mDurationRectangle->setSoundCacheHandler(nullptr);
    }
    mSound->setSoundDataHandler(soundHandler);
    mSound->SWT_setVisible(soundHandler);
    mSound->setVisibile(soundHandler);
}
