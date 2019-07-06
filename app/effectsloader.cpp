#include "effectsloader.h"
#include "GUI/ColorWidgets/colorwidgetshaders.h"
#include "GPUEffects/gpurastereffect.h"
#include <QFileSystemWatcher>
#include <QFileSystemModel>

EffectsLoader::EffectsLoader() {}

void EffectsLoader::initialize() {
    OffscreenQGL33c::initialize();
    try {
        makeCurrent();

        try {
            iniPlainVShaderVBO(this);
            iniPlainVShaderVAO(this, mPlainSquareVAO);
            iniTexturedVShaderVBO(this);
            iniTexturedVShaderVAO(this, mTexturedSquareVAO);
            iniColorPrograms(this);
        } catch(...) {
            RuntimeThrow("Error initializing basic OpenGL programs.");
        }

        iniRasterEffectPrograms();
        doneCurrent();
    } catch(const std::exception& e) {
        doneCurrent();
        gPrintExceptionCritical(e);
    }

    iniCustomPathEffects();
}

#include "PathEffects/custompatheffect.h"
#include "PathEffects/custompatheffectcreator.h"
void iniCustomPathEffect(const QString& path) {
    try {
        CustomPathEffectCreator::sLoadCustomPathEffect(path);
    } catch(...) {
        RuntimeThrow("Error while loading PathEffect from '" + path + "'");
    }
}

void iniIfCustomPathEffect(const QString& path) {
    const QFileInfo fileInfo(path);
    if(!fileInfo.isFile()) return;
    if(!fileInfo.completeSuffix().contains("so")) return;
    try {
        iniCustomPathEffect(path);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

void EffectsLoader::iniCustomPathEffects() {
    const QString dirPath = QDir::homePath() + "/.enve/PathEffects";
    QDirIterator dirIt(dirPath, QDirIterator::NoIteratorFlags);
    while(dirIt.hasNext()) {
        iniIfCustomPathEffect(dirIt.next());
    }
    const auto newFileWatcher = QSharedPointer<QFileSystemModel>(
                new QFileSystemModel);
    newFileWatcher->setRootPath(dirPath);
    connect(newFileWatcher.get(), &QFileSystemModel::rowsInserted, this,
    [newFileWatcher](const QModelIndex &parent, int first, int last) {
        for(int row = first; row <= last; row++) {
            const auto rowIndex = newFileWatcher->index(row, 0, parent);
            const QString path = newFileWatcher->filePath(rowIndex);
            iniIfCustomPathEffect(path);
        }
    });
}

void EffectsLoader::reloadProgram(ShaderEffectCreator* const loaded,
                                  const QString &fragPath) {
    try {
        makeCurrent();
        loaded->reloadProgram(this, fragPath);
        emit programChanged(&loaded->fProgram);
        doneCurrent();
    } catch(const std::exception& e) {
        doneCurrent();
        gPrintExceptionCritical(e);
    }
}

void EffectsLoader::iniRasterEffectPrograms() {
    const QString dirPath = QDir::homePath() + "/.enve/GPURasterEffects";
    QDirIterator dirIt(dirPath, QDirIterator::NoIteratorFlags);

    while(dirIt.hasNext()) {
        const QString path = dirIt.next();
        const QFileInfo fileInfo(path);
        if(!fileInfo.isFile()) continue;
        if(fileInfo.suffix() != "gre") continue;
        try {
            iniRasterEffectProgramExec(path);
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
        }
    }

    const auto newFileWatcher = QSharedPointer<QFileSystemModel>(
                new QFileSystemModel);
    newFileWatcher->setRootPath(dirPath);
    connect(newFileWatcher.get(), &QFileSystemModel::directoryLoaded,
            this, [this, newFileWatcher]() {
        connect(newFileWatcher.get(), &QFileSystemModel::rowsInserted, this,
        [this, newFileWatcher](const QModelIndex &parent, int first, int last) {
            for(int row = first; row <= last; row++) {
                const auto rowIndex = newFileWatcher->index(row, 0, parent);
                const QString path = newFileWatcher->filePath(rowIndex);
                const QFileInfo fileInfo(path);
                if(!fileInfo.isFile()) return;
                const QString suffix = fileInfo.suffix();
                QString grePath;
                QString fragPath;
                if(suffix == "gre") {
                    fragPath = fileInfo.path() + "/" +
                            fileInfo.completeBaseName() + ".frag";
                    if(!QFile(fragPath).exists()) continue;
                    grePath = path;
                } else if(suffix == "frag") {
                    grePath = fileInfo.path() + "/" +
                            fileInfo.completeBaseName() + ".gre";
                    fragPath = path;
                    if(!QFile(grePath).exists()) continue;
                } else continue;
                if(mLoadedGREPaths.contains(grePath)) continue;
                iniSingleRasterEffectProgram(grePath);
            }
        });
    });
}

void EffectsLoader::iniSingleRasterEffectProgram(const QString& grePath) {
    try {
        makeCurrent();
        iniRasterEffectProgramExec(grePath);
        doneCurrent();
    } catch(const std::exception& e) {
        doneCurrent();
        gPrintExceptionCritical(e);
    }
}

void EffectsLoader::iniRasterEffectProgramExec(const QString& grePath) {
    if(!QFile(grePath).exists()) return;
    const QFileInfo fileInfo(grePath);
    const QString fragPath = fileInfo.path() + "/" +
            fileInfo.completeBaseName() + ".frag";
    if(!QFile(fragPath).exists()) return;
    try {
        const auto loaded = ShaderEffectCreator::sLoadFromFile(this, grePath).get();
        mLoadedGREPaths << grePath;

        const auto newFileWatcher = QSharedPointer<QFileSystemWatcher>(
                    new QFileSystemWatcher);
        newFileWatcher->addPath(fragPath);
        connect(newFileWatcher.get(), &QFileSystemWatcher::fileChanged,
                this, [this, loaded, fragPath, newFileWatcher]() {
            reloadProgram(loaded, fragPath);
        });
    } catch(...) {
        const auto newFileWatcher = QSharedPointer<QFileSystemWatcher>(
                    new QFileSystemWatcher);
        newFileWatcher->addPath(grePath);
        newFileWatcher->addPath(fragPath);
        connect(newFileWatcher.get(), &QFileSystemWatcher::fileChanged,
                this, [this, grePath]() {
            iniSingleRasterEffectProgram(grePath);
        });
        RuntimeThrow("Error while loading GPURasterEffect from '" + grePath + "'");
    }
}
