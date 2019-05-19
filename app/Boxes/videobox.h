#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include <unordered_map>
#include "Boxes/animationbox.h"

class SingleSound;

class VideoBox : public AnimationBox {
    friend class SelfRef;
protected:
    VideoBox();
public:
    ~VideoBox();

    void setParentGroup(ContainerBox * const parent);
    void reloadSound();

    bool SWT_isVideoBox() const { return true; }
    void changeSourceFile(QWidget* dialogParent);

    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void setStretch(const qreal& stretch);

    void enableSound() { setSoundEnabled(true); }
    void disableSound() { setSoundEnabled(false); }

    void setSoundEnabled(const bool& enable);

    void setFilePath(const QString &path);
private:
    bool mSoundEnabled = true;
    qsptr<SingleSound> mSound;
    QString mSrcFilePath;
};

#endif // VIDEOBOX_H
