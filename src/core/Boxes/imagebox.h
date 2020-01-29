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

#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include "skia/skiaincludes.h"
#include "FileCacheHandlers/imagecachehandler.h"
#include "imagerenderdata.h"
#include "FileCacheHandlers/filehandlerobjref.h"

struct ImageBoxRenderData : public ImageRenderData {
    ImageBoxRenderData(ImageFileHandler * const cacheHandler,
                       BoundingBox * const parentBoxT) :
        ImageRenderData(parentBoxT) {
        fSrcCacheHandler = cacheHandler;
    }

    void loadImageFromHandler();

    qptr<ImageFileHandler> fSrcCacheHandler;
};

class ImageBox : public BoundingBox {
    e_OBJECT
protected:
    ImageBox();
    ImageBox(const QString &filePath);
public:
    bool SWT_isImageBox() const { return true; }
    void setupCanvasMenu(PropertyMenu * const menu);

    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data,
                         Canvas * const scene);
    stdsptr<BoxRenderData> createRenderData();
    void writeBoundingBox(eWriteStream& dst) const;
    void readBoundingBox(eReadStream& src);

    void changeSourceFile();
    void setFilePath(const QString &path);

    void reload();
private:
    void fileHandlerConnector(ConnContext& conn, ImageFileHandler* obj);
    void fileHandlerAfterAssigned(ImageFileHandler* obj);

    FileHandlerObjRef<ImageFileHandler> mFileHandler;
};

#endif // IMAGEBOX_H
