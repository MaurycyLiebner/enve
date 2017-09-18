#ifndef TILESDATA_H
#define TILESDATA_H

#include "Boxes/boundingboxrendercontainer.h"
#include "updatable.h"
#include <QTemporaryFile>
class Tile;
struct TileSkDrawer;

class TilesData : public MinimalCacheContainer {
public:
    TilesData(const ushort &width,
              const ushort &height,
              const bool &paintInOtherThread);

    ~TilesData();

    void duplicateTilesContentFrom(Tile ***src);

    void clearTiles();

    void clearTmp();

    void getTileDrawers(QList<TileSkDrawer*> *tileDrawers);

    void getTileDrawers(QList<TileSkDrawer*> *tileDrawers,
                        const int &alpha);

    void saveToTmp();

    void drawSk(SkCanvas *canvas, SkPaint *paint);

    void setSize(const ushort &width_t,
                 const ushort &height_t);

    Tile ***getData();
    void writeTilesData(QFile *file);
    void readTilesData(QFile *file);

    void setCurrentlyUsed(const bool &used);

    void dataChanged();

    bool freeThis();

    int getByteCount();

    void loadDataFromTmpFile();
    void afterLoadedFromTmpFile();

    void afterSavedToTmpFile();
    void _writeTilesDataToTmp();
    void _readTilesDataFromTmp();
    void initializeEmptyTilesData();
private:
    bool mNoDataInMemory = true;
    bool mDataStoredInTmpFile = false;
    QTemporaryFile *mTmpFile = NULL;

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
    Tile ***mTiles = NULL;
};

class SaveTilesDataToTmpFileExec : public Executable {
public:
    SaveTilesDataToTmpFileExec(TilesData *targetTilesDataT) {
        mTargetTilesData = targetTilesDataT;
    }

    void processUpdate() {
        mTargetTilesData->_writeTilesDataToTmp();
    }

    void afterUpdate() {
        mTargetTilesData->afterSavedToTmpFile();
    }

private:
    TilesData *mTargetTilesData;
};

class LoadTilesDataFromTmpFileExec : public Executable {
public:
    LoadTilesDataFromTmpFileExec(TilesData *targetTilesDataT) {
        mTargetTilesData = targetTilesDataT;
    }

    void processUpdate() {
        mTargetTilesData->_readTilesDataFromTmp();
    }


    void afterUpdate() {
        mTargetTilesData->afterLoadedFromTmpFile();
    }

private:
    TilesData *mTargetTilesData;
};
#endif // TILESDATA_H
