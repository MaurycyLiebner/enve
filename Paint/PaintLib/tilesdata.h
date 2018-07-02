#ifndef TILESDATA_H
#define TILESDATA_H

#include "Boxes/boundingboxrendercontainer.h"
#include "updatable.h"
#include <QTemporaryFile>
#include <QAction>
class Tile;
struct TileSkDrawer;
struct TileSkDrawerCollection;

class TilesData : public MinimalCacheContainer {
public:
    TilesData(const ushort &width,
              const ushort &height,
              const bool &paintInOtherThread);

    ~TilesData();

    void duplicateTilesContentFrom(TilesData *src);

    void clearTiles();

    void clearTmp();

    void getTileDrawers(QList<TileSkDrawer*> *tileDrawers);

    void saveToTmp();

    void drawSk(SkCanvas *canvas, SkPaint *paint);

    void setSize(const ushort &width_t,
                 const ushort &height_t);

    Tile ***getTiles();
    bool writeTilesData(QIODevice *target);
    void readTilesData(QIODevice *target);

    void setCurrentlyUsed(const bool &used);

    void dataChanged();

    bool cacheAndFree();
    bool freeAndRemove();

    int getByteCount();

    void loadDataFromTmpFile();
    void afterLoadedFromTmpFile();

    void afterSavedToTmpFile();
    bool _writeTilesDataToTmp();
    void _readTilesDataFromTmp();
    void initializeEmptyTilesData();
    void writeTilesDataFromMemoryOrTmp(QIODevice *target);
    void move(const int &xT, const int &yT);

    void setImage(const QImage &img);

    void incNumberTilesBeingProcessed() {
        mNTilesBeingProcessed++;
    }

    void decNumberTilesBeingProcessed() {
        mNTilesBeingProcessed--;
        if(mNTilesBeingProcessed == 0) {
            processAwaitingOperations();
        }
    }

    bool tilesBeingProcessed() {
        return mNTilesBeingProcessed > 0;
    }

    void addChangedTile(Tile *tile) {
        mTilesChanged << tile;
    }

    void finishTransform();
private:
    QList<Tile*> mTilesChanged;
    void processAwaitingOperations() {

    }
    int mNTilesBeingProcessed = 0;
    void replaceData(const int &srcX, const int &srcY,
                     const int &targetX, const int &targetY,
                     const int &width, const int &height,
                     const int &srcTileX, const int &srcTileY,
                     Tile *targetTile);
    bool mNoDataInMemory = true;
    bool mDataStoredInTmpFile = false;
    QTemporaryFile *mTmpFile = nullptr;

    bool mCurrentlyUsed = false;
    void resizeTiles(const ushort &nTileCols,
                     const ushort &nTilesRows,
                     const ushort &lastColumnWidth,
                     const ushort &lastRowHeight);

    bool mPaintInOtherThread;
    ushort mWidth = 0;
    ushort mHeight = 0;
    ushort mNTileCols = 0;
    ushort mNTileRows = 0;
    ushort mLastRowHeight = 0;
    ushort mLastColumnWidth = 0;
    Tile ***mTiles = nullptr;
};

class SaveTilesDataToTmpFileExec : public _Executor {
public:
    SaveTilesDataToTmpFileExec(TilesData *targetTilesDataT) {
        mTargetTilesData = targetTilesDataT;
    }

    void _processUpdate() {
        mTargetTilesData->_writeTilesDataToTmp();
    }

    void afterUpdate() {
        mTargetTilesData->afterSavedToTmpFile();
    }

private:
    TilesData *mTargetTilesData;
};

class LoadTilesDataFromTmpFileExec : public _Executor {
public:
    LoadTilesDataFromTmpFileExec(TilesData *targetTilesDataT) {
        mTargetTilesData = targetTilesDataT;
    }

    void _processUpdate() {
        mTargetTilesData->_readTilesDataFromTmp();
    }


    void afterUpdate() {
        mTargetTilesData->afterLoadedFromTmpFile();
    }

private:
    TilesData *mTargetTilesData;
};
#endif // TILESDATA_H
