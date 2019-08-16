#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include <unordered_map>
#include "Boxes/animationbox.h"
#include "FileCacheHandlers/videocachehandler.h"

class SingleSound;

class VideoBox : public AnimationBox {
    e_OBJECT
protected:
    VideoBox();
public:
    void animationDataChanged();
    void soundDataChanged();

    bool SWT_isVideoBox() const { return true; }
    void changeSourceFile(QWidget * const dialogParent);

    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void setStretch(const qreal stretch);

    void setFilePath(const QString& path);

    void enableSound() { setSoundEnabled(true); }
    void disableSound() { setSoundEnabled(false); }

    void setSoundEnabled(const bool enable);
private:
    bool mSoundEnabled = true;
    qsptr<SingleSound> mSound;
    qptr<VideoFileHandler> mFileHandler;
};

#endif // VIDEOBOX_H
