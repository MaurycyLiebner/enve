#ifndef SOUNDCOMPOSITION_H
#define SOUNDCOMPOSITION_H

#include "Animators/complexanimator.h"

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
#include "singlesound.h"
const int SAMPLERATE = 44100;

class SoundComposition : public QIODevice
{
    Q_OBJECT
public:
    SoundComposition(QObject *parent);
    ~SoundComposition();

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

    void generateData(const int &startFrame,
                      const int &endFrame,
                      const qreal &fps);

    void addSound(SingleSound *sound);
    void removeSound(SingleSound *sound);

    ComplexAnimator *getSoundsAnimatorContainer();
    void addSoundAnimator(SingleSound *sound);
    void removeSoundAnimator(SingleSound *sound);
private:
    ComplexAnimator mSoundsAnimatorContainer;
    QList<SingleSound*> mSounds;
    qint64 mPos;
    QByteArray mBuffer;
};

#endif // SOUNDCOMPOSITION_H
