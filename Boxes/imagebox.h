#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include "skiaincludes.h"
class FileCacheHandler;
class ImageCacheHandler;

struct ImageBoxRenderData : public BoundingBoxRenderData {
    ImageBoxRenderData(FileCacheHandler *cacheHandler,
                       BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT){
        srcCacheHandler = cacheHandler;
    }

    virtual void loadImageFromHandler();

    void beforeUpdate() {
        if(!allDataReady()) {
            loadImageFromHandler();
        }
        BoundingBoxRenderData::beforeUpdate();
    }

    bool allDataReady() {
        return image.get() != NULL;
    }

    void updateRelBoundingRect() {
        relBoundingRect = QRectF(0., 0., image->width(), image->height());
    }

    sk_sp<SkImage> image;

    FileCacheHandler *srcCacheHandler;
private:
    void drawSk(SkCanvas *canvas) {
        SkPaint paint;
        //paint.setFilterQuality(kHigh_SkFilterQuality);
        canvas->drawImage(image, 0, 0, &paint);
    }
};

class ImageBox : public BoundingBox {
public:
    ImageBox(QString filePath = "");
    ~ImageBox();

    void setFilePath(const QString &path);

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate();

    bool SWT_isImageBox() { return true; }
    void addActionsToMenu(QMenu *menu);
    bool handleSelectedCanvasAction(QAction *selectedAction);
    void changeSourceFile();

    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);

    BoundingBoxRenderData *createRenderData();

    void loadFromSql(const int &boundingBoxId);
    int saveToSql(QSqlQuery *query, const int &parentId);
private:
    ImageCacheHandler *mImgCacheHandler = NULL;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
