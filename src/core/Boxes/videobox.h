// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

    void writeBoundingBox(eWriteStream& dst);
    void readBoundingBox(eReadStream& src);

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
