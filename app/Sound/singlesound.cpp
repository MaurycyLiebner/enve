#include "singlesound.h"
#include "soundcomposition.h"
#include "durationrectangle.h"
#include "PropertyUpdaters/singlesoundupdater.h"

extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}

int decode_audio_file(const char* path,
                      const int sample_rate,
                      float** audioData,
                      int* size) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, nullptr, nullptr) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return -1;
    }
    if (avformat_find_stream_info(format, nullptr) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    int audioStreamIndex = -1;
    for(uint i = 0; i< format->nb_streams; i++) {
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }
    if (audioStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve audio stream from file '%s'\n", path);
        return -1;
    }
    AVCodecContext *audioCodec = nullptr;
    struct SwrContext *swr = nullptr;

    AVStream* audioStream = format->streams[audioStreamIndex];
    // find & open codec
    audioCodec = audioStream->codec;
    if (avcodec_open2(audioCodec, avcodec_find_decoder(audioCodec->codec_id), nullptr) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n",
                audioStreamIndex, path);
        return -1;
    }

    // prepare resampler
    swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  audioCodec->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 1, 0);
    av_opt_set_int(swr, "in_channel_layout",  audioCodec->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(swr, "in_sample_rate", audioCodec->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  audioCodec->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT,  0);
    swr_init(swr);
    if (!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        return -1;
    }

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Error allocating the frame\n");
        return -1;
    }

    // iterate through frames
    *audioData = nullptr;
    *size = 0;
    while(av_read_frame(format, &packet) >= 0) {
        if(packet.stream_index == audioStreamIndex) {
            // decode one frame
            int gotFrame;
            if(avcodec_decode_audio4(audioCodec, frame, &gotFrame, &packet) < 0) {
                break;
            }
            if(!gotFrame) {
                continue;
            }
            // resample frames
            float *buffer;
            av_samples_alloc((uint8_t**) &buffer, nullptr, 1,
                             frame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
            int frame_count = swr_convert(swr,
                                          (uint8_t**) &buffer,
                                          frame->nb_samples,
                                          (const uint8_t**) frame->data,
                                          frame->nb_samples);
            // append resampled frames to data
            *audioData = (float*) realloc(*audioData,
                                     (*size + frame->nb_samples) * sizeof(float));
            memcpy(*audioData + *size, buffer, frame_count * sizeof(float));

            av_freep(&((uint8_t**) &buffer)[0]);
            *size += frame_count;
        }
        av_free_packet(&packet);
    }

    // clean up
    av_frame_free(&frame);
    if(swr != nullptr) {
        swr_free(&swr);
    }
    if(audioCodec != nullptr) {
        avcodec_close(audioCodec);
    }
    avformat_free_context(format);

    // success
    return 0;
}

SingleSound::SingleSound(const QString &path,
                         FixedLenAnimationRect *durRect) :
    ComplexAnimator("sound") {
    setDurationRect(durRect);

    prp_setUpdater(SPtrCreate(SingleSoundUpdater)(this));
    prp_blockUpdater();

    ca_addChildAnimator(mVolumeAnimator);
    mVolumeAnimator->qra_setValueRange(0, 200);
    mVolumeAnimator->qra_setCurrentValue(100);

    setFilePath(path);
}

void SingleSound::prp_drawKeys(QPainter *p,
                               const qreal &pixelsPerFrame,
                               const qreal &drawY,
                               const int &startFrame,
                               const int &endFrame,
                               const int &rowHeight,
                               const int &keyRectSize) {
//    qreal timeScale = mTimeScaleAnimator.getCurrentValue();
//    int startDFrame = mDurationRectangle.getMinAnimationFrame() - startFrame;
//    int frameWidth = ceil(mListOfFrames.count()/qAbs(timeScale));
//    p->fillRect(startDFrame*pixelsPerFrame + pixelsPerFrame*0.5, drawY,
//                frameWidth*pixelsPerFrame - pixelsPerFrame,
//                BOX_HEIGHT, QColor(0, 0, 255, 125));
    mDurationRectangle->draw(p, pixelsPerFrame,
                             drawY, startFrame);
    ComplexAnimator::prp_drawKeys(p, pixelsPerFrame, drawY,
                                  startFrame, endFrame,
                                  rowHeight, keyRectSize);
}

FixedLenAnimationRect *SingleSound::getDurationRect() {
    return mDurationRectangle;
}

void SingleSound::setDurationRect(FixedLenAnimationRect *durRect) {
    if(mDurationRectangle != nullptr) {
        delete mDurationRectangle;
    }
    if(durRect == nullptr) {
        mOwnDurationRectangle = true;
        mDurationRectangle = new FixedLenAnimationRect(this);
        mDurationRectangle->setBindToAnimationFrameRange();
        connect(mDurationRectangle, SIGNAL(posChanged(int)),
                this, SLOT(anim_updateAfterShifted()));
    } else {
        mOwnDurationRectangle = false;
        mDurationRectangle = durRect;
    }
    connect(mDurationRectangle, SIGNAL(rangeChanged()),
            this, SLOT(scheduleFinalDataUpdate()));
    connect(mDurationRectangle, SIGNAL(posChanged(int)),
            this, SLOT(updateAfterDurationRectangleShifted()));
}

void SingleSound::updateAfterDurationRectangleShifted() {
    prp_setParentFrameShift(prp_mParentFrameShift);
    prp_setAbsFrame(anim_mCurrentAbsFrame);
    scheduleFinalDataUpdate();
}

DurationRectangleMovable *SingleSound::anim_getRectangleMovableAtPos(
                            const int &relX,
                            const int &minViewedFrame,
                            const qreal &pixelsPerFrame) {
    return mDurationRectangle->getMovableAt(relX, pixelsPerFrame,
                                            minViewedFrame);
}

void SingleSound::updateFinalDataIfNeeded(const qreal &fps,
                                          const int &minAbsFrame,
                                          const int &maxAbsFrame) {
    if(mFinalDataUpdateNeeded) {
        prepareFinalData(static_cast<float>(fps), minAbsFrame, maxAbsFrame);
        mFinalDataUpdateNeeded = false;
    }
}

void SingleSound::scheduleFinalDataUpdate() {
    mFinalDataUpdateNeeded = true;
}


void SingleSound::setFilePath(const QString &path) {
    mPath = path;
    reloadDataFromFile();
}

void SingleSound::reloadDataFromFile() {
    if(mSrcData != nullptr) {
        free(mSrcData);
        mSrcData = nullptr;
        mSrcSampleCount = 0;
    }
    if(!mPath.isEmpty()) {
        if(QFile(mPath).exists()) {
            decode_audio_file(mPath.toLatin1().data(), SOUND_SAMPLERATE,
                              &mSrcData, &mSrcSampleCount);
        }
    }
    if(mOwnDurationRectangle) {
        mDurationRectangle->setAnimationFrameDuration(
                    qCeil(mSrcSampleCount*24./SOUND_SAMPLERATE));
    }

    scheduleFinalDataUpdate();
}

int SingleSound::getStartAbsFrame() const {
    return mFinalAbsStartFrame;
}

int SingleSound::getSampleCount() const {
    return mFinalSampleCount;
}

void SingleSound::prepareFinalData(const float &fps,
                                   const int &minAbsFrame,
                                   const int &maxAbsFrame) {
    if(mFinalData != nullptr) {
        free(mFinalData);
    }
    if(mSrcData == nullptr) {
        mFinalData = nullptr;
        mFinalSampleCount = 0;
    } else {
        mFinalAbsStartFrame =
            qMax(minAbsFrame,
                   qMax(mDurationRectangle->getMinFrameAsAbsFrame(),
                        mDurationRectangle->getMinAnimationFrameAsAbsFrame()) );
        int finalMaxAbsFrame =
            qMin(maxAbsFrame,
                   qMin(mDurationRectangle->getMaxFrameAsAbsFrame(),
                        mDurationRectangle->getMaxAnimationFrameAsAbsFrame()) );
        qDebug() << "sound abs frame range:" << mFinalAbsStartFrame << finalMaxAbsFrame;
        int finalMinRelFrame = prp_absFrameToRelFrame(mFinalAbsStartFrame);
        int finalMaxRelFrame = prp_absFrameToRelFrame(finalMaxAbsFrame);
        qDebug() << "sound rel frame range:" << finalMinRelFrame << finalMaxRelFrame;

        int minSampleFromSrc = static_cast<int>(
                    floor(finalMinRelFrame*SOUND_SAMPLERATE/fps));
        int maxSamplePlayed = static_cast<int>(
                    ceil(finalMaxRelFrame*SOUND_SAMPLERATE/fps));
        int maxSampleFromSrc = std::min(mSrcSampleCount, maxSamplePlayed);


        mFinalSampleCount = static_cast<int>(maxSampleFromSrc - minSampleFromSrc);
        qDebug() << minSampleFromSrc << maxSampleFromSrc << mFinalSampleCount;
        ulong dataSize = static_cast<ulong>(mFinalSampleCount)*sizeof(float);
        void* data = malloc(dataSize);
        mFinalData = static_cast<float*>(data);
        if(mVolumeAnimator->prp_hasKeys()) {
            int j = 0;
            int frame = 0;
            qreal volVal = mVolumeAnimator->qra_getEffectiveValueAtRelFrame(frame);
            float lastFrameVol = static_cast<float>(volVal*0.01);
            float volStep = fps/SOUND_SAMPLERATE;
            while(j < mFinalSampleCount) {
                frame++;
                float nextFrameVol = static_cast<float>(
                        mVolumeAnimator->qra_getEffectiveValueAtRelFrame(frame)/100.);
                float volDiff = (nextFrameVol - lastFrameVol);
                float currVolFrac = lastFrameVol;
                for(int i = 0;
                    i < SOUND_SAMPLERATE/fps && j < mFinalSampleCount;
                    i++, j++) {
                    currVolFrac += volStep*volDiff;
                    mFinalData[j] = static_cast<float>(mSrcData[j + minSampleFromSrc]*
                                            currVolFrac);
                }
                lastFrameVol = nextFrameVol;
            }
        } else {
            float volFrac =
                    static_cast<float>(mVolumeAnimator->qra_getCurrentValue()/100.);
            for(int i = 0; i < mFinalSampleCount; i++) {
                mFinalData[i] = mSrcData[i + minSampleFromSrc]*volFrac;
            }
        }
    }
}

int SingleSound::prp_getFrameShift() const {
    if(mOwnDurationRectangle) {
        return mDurationRectangle->getFrameShift() +
                Animator::prp_getFrameShift();
    }
    return Animator::prp_getFrameShift();
}

#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
bool SingleSound::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool &parentSatisfies,
                                      const bool &parentMainTarget) {
    if(rules.type == &SingleWidgetTarget::SWT_isSingleSound) return true;
    return SingleWidgetTarget::SWT_shouldBeVisible(rules,
                                                   parentSatisfies,
                                                   parentMainTarget);
}

const float *SingleSound::getFinalData() const {
    return mFinalData;
}
