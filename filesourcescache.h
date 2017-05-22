#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <unordered_map>
#include <QString>
#include <QList>
class FileCacheHandler;

class FileSourcesCache
{
public:
    FileSourcesCache();

    void addHandler(FileCacheHandler *handlerPtr);
    FileCacheHandler *getHandlerForFilePath(const QString &filePath);
    void removeHandler(FileCacheHandler *handler);
private:
    QList<FileCacheHandler*> mFileCacheHandlers;
};

#endif // FILESOURCESCACHE_H
