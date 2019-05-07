#ifndef SOUNDCOMPOSITION_H
#define SOUNDCOMPOSITION_H

#include "Animators/complexanimator.h"
#include "CacheHandlers/hddcachablecachehandler.h"
#include "CacheHandlers/samples.h"

#include <math.h>

#include <QAudioOutput>
#include <QByteArray>
#include <QComboBox>
#include <QIODevice>
#include <QLabel>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
class SingleSound;
class Canvas;
class SoundMerger;
const int SOUND_SAMPLERATE = 44100;

class SoundComposition : public QIODevice {
public:
    SoundComposition(Canvas * const parent);

    void start(const int &startFrame);
    void stop();

    qint64 readData(char *data, qint64 maxLen);
    qint64 writeData(const char *data, qint64 len);

    void addSound(const qsptr<SingleSound> &sound);
    void removeSound(const qsptr<SingleSound> &sound);

    ComplexAnimator *getSoundsAnimatorContainer();
    void addSoundAnimator(const qsptr<SingleSound> &sound);
    void removeSoundAnimator(const qsptr<SingleSound> &sound);

    void secondFinished(const int& secondId,
                        const stdsptr<Samples>& samples);

    SoundMerger * scheduleFrame(const int& frameId);
private:
    SoundMerger * scheduleSecond(const int& secondId);

    void frameRangeChanged(const FrameRange &range);

    void secondRangeChanged(const iValueRange &range) {
        mSecondsCache.clearRelRange(range);
    }

    QList<int> mProcessingSeconds;
    const Canvas * const mParent;
    qint64 mPos;
    qsptr<ComplexAnimator> mSoundsAnimatorContainer =
            SPtrCreate(ComplexAnimator)("sounds");
    QList<qsptr<SingleSound>> mSounds;
    HDDCachableCacheHandler mSecondsCache;
};

#endif // SOUNDCOMPOSITION_H
