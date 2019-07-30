#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include "skia/skiaincludes.h"
#include "FileCacheHandlers/imagecachehandler.h"
class FileDataCacheHandler;

struct ImageRenderData : public BoxRenderData {
    ImageRenderData(BoundingBox * const parentBoxT) :
        BoxRenderData(parentBoxT) {
        mDelayDataSet = true;
    }

    virtual void loadImageFromHandler() = 0;

    void updateRelBoundingRect() {
        if(fImage) fRelBoundingRect =
                QRectF(0, 0, fImage->width(), fImage->height());
        else fRelBoundingRect = QRectF(0, 0, 0, 0);
    }

    void setupRenderData() final {
        if(!fImage) loadImageFromHandler();
        if(!hasEffects()) setupDirectDraw();
    }

    sk_sp<SkImage> fImage;
private:
    void setupDirectDraw() {
        fBaseMargin = QMargins();
        updateRelBoundingRect();
        updateGlobalRect();
        //fRenderTransform = fScaledTransform;
        //fRenderTransform.translate(-fGlobalRect.x(), -fGlobalRect.y());
        fRenderedImage = fImage;
        finishedProcessing();
    }

    void drawSk(SkCanvas * const canvas) {
        SkPaint paint;
        //paint.setFilterQuality(kHigh_SkFilterQuality);
        //paint.setAntiAlias(true);
        if(fImage) canvas->drawImage(fImage,
                                     toSkScalar(fRelBoundingRect.x()),
                                     toSkScalar(fRelBoundingRect.y()),
                                     &paint);
    }
};

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
    friend class SelfRef;
protected:
    ImageBox();
    ImageBox(const QString &filePath);
public:
    bool SWT_isImageBox() const { return true; }
    void setupCanvasMenu(PropertyMenu * const menu);

    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data);
    stdsptr<BoxRenderData> createRenderData();
    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void changeSourceFile(QWidget * const dialogParent);
    void setFilePath(const QString &path);

    void reload();
private:
    ImageFileHandler* mImgCacheHandler;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
