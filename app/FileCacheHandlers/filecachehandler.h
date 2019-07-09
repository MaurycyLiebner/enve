#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "smartPointers/stdselfref.h"
#include "filedatacachehandler.h"
#include "smartPointers/sharedpointerdefs.h"

class FileCacheHandler : public StdSelfRef {
protected:
    FileCacheHandler(const QString& name);
public:
    ~FileCacheHandler();

    virtual void reload() = 0;
    virtual void replace() = 0;

    const QString& name() const { return mName; }
    bool fileMissing() const { return mFileMissing; }

    static FileCacheHandler *sGetFileHandler(const QString &filePath);
private:
    static QList<FileCacheHandler*> sFileHandlers;
    const QString mName; // Usually filename
    bool mFileMissing = false;
};



#endif // FILECACHEHANDLER_H
