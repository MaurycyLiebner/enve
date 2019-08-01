#ifndef IMPORTHANDLER_H
#define IMPORTHANDLER_H
#include <QFileInfo>
#include <memory>
#include "Boxes/boundingbox.h"

class eImporter {
public:
    virtual bool supports(const QFileInfo& fileInfo) const = 0;
    //! @brief Can return nullptr.
    virtual qsptr<BoundingBox> import(const QFileInfo& fileInfo) const = 0;
private:
};

class ImportHandler {
public:
    ImportHandler();

    static ImportHandler* sInstance;

    template<typename T>
    void addImporter() {
        mImporters << std::make_shared<T>();
    }

    qsptr<BoundingBox> import(const QString& path) const;
private:
    QList<std::shared_ptr<eImporter>> mImporters;
};

#endif // IMPORTHANDLER_H
