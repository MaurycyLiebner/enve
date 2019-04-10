#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include "skia/skiaincludes.h"
class FileCacheHandler;
class ImageCacheHandler;

struct ImageBoxRenderData : public BoundingBoxRenderData {
    ImageBoxRenderData(FileCacheHandler * const cacheHandler,
                       BoundingBox * const parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
        mDelayDataSet = true;
        fSrcCacheHandler = cacheHandler;
    }

    virtual void loadImageFromHandler();

    void beforeProcessingStarted() {
        if(!fImage) loadImageFromHandler();
        BoundingBoxRenderData::beforeProcessingStarted();
    }

    void updateRelBoundingRect() {
        if(fImage) fRelBoundingRect =
                QRectF(0, 0, fImage->width(), fImage->height());
        else fRelBoundingRect = QRectF(0, 0, 0, 0);
    }

    sk_sp<SkImage> fImage;

    FileCacheHandler *fSrcCacheHandler;
private:
    void drawSk(SkCanvas * const canvas) {
        SkPaint paint;
        //paint.setFilterQuality(kHigh_SkFilterQuality);
        //paint.setAntiAlias(true);
        if(fImage) canvas->drawImage(fImage, 0, 0, &paint);
    }
};

class ImageBox : public BoundingBox {
    friend class SelfRef;
protected:
    ImageBox();
    ImageBox(const QString &filePath);
public:
    ~ImageBox();

    bool SWT_isImageBox() const { return true; }
    void addActionsToMenu(BoxTypeMenu * const menu);

    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData* data);
    stdsptr<BoundingBoxRenderData> createRenderData();
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);

    void changeSourceFile(QWidget *dialogParent);
    void setFilePath(const QString &path);

    void reload();
private:
    stdptr<ImageCacheHandler> mImgCacheHandler;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
