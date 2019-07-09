#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include "skia/skiaincludes.h"
class FileDataCacheHandler;
class ImageCacheHandler;

struct ImageRenderData : public BoundingBoxRenderData {
    ImageRenderData(BoundingBox * const parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
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
        if(fRasterEffects.isEmpty() &&
           fGPUEffects.isEmpty()) {
            setupDirectDraw();
        }
    }

    sk_sp<SkImage> fImage;
private:
    void setupDirectDraw() {
        updateRelBoundingRect();
        updateGlobalFromRelBoundingRect();
        //fRenderTransform = fScaledTransform;
        //fRenderTransform.translate(-fDrawPos.x(), -fDrawPos.y());
        fRenderedImage = fImage;
    }

    void drawSk(SkCanvas * const canvas) {
        SkPaint paint;
        //paint.setFilterQuality(kHigh_SkFilterQuality);
        //paint.setAntiAlias(true);
        if(fImage) canvas->drawImage(fImage,
                                     qRound(fRelBoundingRect.x()),
                                     qRound(fRelBoundingRect.y()), &paint);
    }
};

struct ImageBoxRenderData : public ImageRenderData {
    ImageBoxRenderData(ImageCacheHandler * const cacheHandler,
                       BoundingBox * const parentBoxT) :
        ImageRenderData(parentBoxT) {
        mDelayDataSet = true;
        fSrcCacheHandler = cacheHandler;
    }

    void loadImageFromHandler();

    ImageCacheHandler *fSrcCacheHandler;
};

class ImageBox : public BoundingBox {
    friend class SelfRef;
protected:
    ImageBox();
    ImageBox(const QString &filePath);
public:
    bool SWT_isImageBox() const { return true; }
    void addActionsToMenu(BoxTypeMenu * const menu);

    void setupRenderData(const qreal relFrame,
                         BoundingBoxRenderData * const data);
    stdsptr<BoundingBoxRenderData> createRenderData();
    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void changeSourceFile(QWidget * const dialogParent);
    void setFilePath(const QString &path);

    void reload();
private:
    ImageCacheHandler* mImgCacheHandler;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
