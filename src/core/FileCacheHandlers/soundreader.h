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

#ifndef SOUNDREADER_H
#define SOUNDREADER_H
#include "Tasks/updatable.h"
#include "CacheHandlers/samples.h"
#include "framerange.h"
#include "../Sound/esoundsettings.h"
extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

class SoundHandler;
struct AudioStreamsData;

class CORE_EXPORT SoundReader : public eHddTask {
    e_OBJECT
protected:
    SoundReader(SoundHandler * const cacheHandler,
                const stdsptr<AudioStreamsData>& openedAudio,
                const int secondId, const SampleRange& sampleRange) :
        mCacheHandler(cacheHandler), mOpenedAudio(openedAudio),
        mSecondId(secondId), mSampleRange(sampleRange),
        mSettings(eSoundSettings::sData()) {}

    void beforeProcessing(const Hardware);
    void afterProcessing();
    void afterCanceled();
public:
    void process() { readFrame(); }
protected:
    const stdsptr<Samples>& getSamples() const {
        return mSamples;
    }
private:
    void readFrame();

    SoundHandler * const mCacheHandler;
    const stdsptr<AudioStreamsData> mOpenedAudio;
    const int mSecondId;
    const SampleRange mSampleRange;
    const eSoundSettingsData mSettings;
    stdsptr<Samples> mSamples;
};

#include "Sound/soundmerger.h"
class CORE_EXPORT SoundReaderForMerger : public SoundReader {
    e_OBJECT
    struct SingleSound {
        int fSampleShift;
        SampleRange fSamplesRange;
        QrealSnapshot fVolume;
        qreal fSpeed;
    };
protected:
    SoundReaderForMerger(SoundHandler * const cacheHandler,
                         const stdsptr<AudioStreamsData>& openedAudio,
                         const int secondId, const SampleRange& sampleRange) :
        SoundReader(cacheHandler, openedAudio, secondId, sampleRange) {}

    void afterProcessing();
public:
    void addSingleSound(const int sampleShift,
                        const SampleRange& absRange,
                        const QrealSnapshot& volume,
                        const qreal speed) {
        mSSAbsRanges.append({sampleShift, absRange, volume, speed});
    }

    void addMerger(SoundMerger * const merger) {
        mMergers << merger;
    }
private:
    QList<SingleSound> mSSAbsRanges;
    QList<stdptr<SoundMerger>> mMergers;
};
#endif // SOUNDREADER_H
