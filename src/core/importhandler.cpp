#include "importhandler.h"

#include "exceptions.h"

ImportHandler* ImportHandler::sInstance = nullptr;

ImportHandler::ImportHandler() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

qsptr<BoundingBox> ImportHandler::import(const QString &path) const {
    {
        const QFile file(path);
        if(!file.exists()) RuntimeThrow("File does not exist");
    }
    const QFileInfo info(path);
    for(const auto& importer : mImporters) {
        if(importer->supports(info)) {
            try {
                return importer->import(info);
            } catch(...) {
                RuntimeThrow("Importer " + typeid(importer.get()).name() + " failed.");
            }
        }
    }
    RuntimeThrow("Unsupported file format:\n'" + path + "'\n");
}
