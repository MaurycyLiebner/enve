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
class SingleSound;
const int SOUND_SAMPLERATE = 44100;

class SoundComposition : public QIODevice {
    Q_OBJECT
public:
    SoundComposition(QObject *parent);

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

    void generateData(const int &startAbsFrame,
                      const int &endAbsFrame,
                      const qreal &fps);

    void addSound(const SingleSoundQSPtr &sound);
    void removeSound(const SingleSoundQSPtr &sound);

    ComplexAnimator *getSoundsAnimatorContainer();
    void addSoundAnimator(const SingleSoundQSPtr &sound);
    void removeSoundAnimator(const SingleSoundQSPtr &sound);
private:
    QByteArray mBuffer;
    qint64 mPos;
    QSharedPointer<ComplexAnimator> mSoundsAnimatorContainer =
            SPtrCreate(ComplexAnimator)("sounds");
    QList<SingleSoundQSPtr> mSounds;
};

#endif // SOUNDCOMPOSITION_H
