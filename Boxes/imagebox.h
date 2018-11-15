#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include "skiaincludes.h"
class FileCacheHandler;
class ImageCacheHandler;

struct ImageBoxRenderData : public BoundingBoxRenderData {
    ImageBoxRenderData(FileCacheHandler *cacheHandler,
                       BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
        mDelayDataSet = true;
        srcCacheHandler = cacheHandler;
    }

    virtual void loadImageFromHandler();

    void beforeUpdate() {
        if(!allDataReady()) {
            loadImageFromHandler();
        }
        BoundingBoxRenderData::beforeUpdate();
        if(image.get() == nullptr) {
            qDebug() << "ImageBoxRenderData::beforeUpdate() - no image to render";
        }
    }

    bool allDataReady() {
        return image.get() != nullptr;
    }

    void updateRelBoundingRect() {
        relBoundingRect = QRectF(0., 0., image->width(), image->height());
    }

    sk_sp<SkImage> image;

    FileCacheHandler *srcCacheHandler;
private:
    void drawSk(SkCanvas *canvas) {
        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality);
        paint.setAntiAlias(true);
        if(image == nullptr) {
            qDebug() << "ImageBoxRenderData::drawSk() - no image to draw";
        }
        canvas->drawImage(image, 0, 0, &paint);
    }
};

class ImageBox : public BoundingBox {
public:
    ImageBox();
    ImageBox(const QString &filePath);
    ~ImageBox();

    void setFilePath(const QString &path);

    bool SWT_isImageBox() { return true; }
    void addActionsToMenu(QMenu *menu);
    bool handleSelectedCanvasAction(QAction *selectedAction);
    void changeSourceFile();

    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                                const std::shared_ptr<BoundingBoxRenderData>& data);
    std::shared_ptr<BoundingBoxRenderData> createRenderData();
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
private:
    ImageCacheHandler *mImgCacheHandler = nullptr;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
