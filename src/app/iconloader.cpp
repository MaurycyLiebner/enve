#include "iconloader.h"

#include <QDir>
#include <QSvgRenderer>
#include <QPainter>

#include "esettings.h"
#include "exceptions.h"

void renderSvg(const QString& svgPath, QImage& image) {
    QSvgRenderer renderer(svgPath);
    QPainter painter(&image);
    renderer.viewBox();
    renderer.render(&painter);
}

bool IconLoader::generate(const QString& svgPath,
                          const qreal scale,
                          const QString& pngPath) {
    if(QFile(pngPath).exists()) return false;

    QSvgRenderer renderer(svgPath);
    const auto defSize = renderer.defaultSize();
    QImage image(qRound(scale*defSize.width()),
                 qRound(scale*defSize.height()),
                 QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    renderer.render(&painter);

    if(!image.save(pngPath)) RuntimeThrow("Failed to save icon file '" + pngPath + "'");
    return true;
}

bool generate(const QString& svgPath,
              const QImage& base,
              const QString& pngPath) {
    if(QFile(pngPath).exists()) return false;

    QImage image = base;
    renderSvg(svgPath, image);

    if(!image.save(pngPath)) RuntimeThrow("Failed to save icon file '" + pngPath + "'");
    return true;
}

void generateButtons(const int minWidgetDim, const int buttonDim) {
    const QSize buttonSize(buttonDim, buttonDim);

    const QDir eDir(eSettings::sSettingsDir());
    const QString mkPath = "icons/" + QString::number(minWidgetDim) + "/toolbarButtons";
    if(!eDir.mkpath(mkPath)) RuntimeThrow("Failed to mkpath '" + mkPath + "'");

    QImage uncheckedBase(buttonSize, QImage::Format_ARGB32);
    uncheckedBase.fill(Qt::transparent);
    renderSvg(":/icons/toolbarButtons/uncheckedBg", uncheckedBase);

    QImage checkedBase(buttonSize, QImage::Format_ARGB32);
    checkedBase.fill(Qt::transparent);
    renderSvg(":/icons/toolbarButtons/checkedBg", checkedBase);

    const QString dir = eSettings::sSettingsDir() + "/" + mkPath;
    QDirIterator checkableIt(":/icons/toolbarButtons/checkable");
    while(checkableIt.hasNext()) {
        const auto path = checkableIt.next();
        const auto fileName = checkableIt.fileName();
        const auto pngFileName = fileName + ".png";

        if(fileName.contains("Checked")) {
            const auto pngPath = dir + "/" + fileName + ".png";
            generate(path, checkedBase, pngPath);
        } else if(fileName.contains("Unchecked")) {
            const auto pngPath = dir + "/" + fileName + ".png";
            generate(path, uncheckedBase, pngPath);
        } else {
            generate(path, uncheckedBase, dir + "/" + fileName + "Unchecked.png");
            generate(path, checkedBase, dir + "/" + fileName + "Checked.png");
        }
    }

    QDirIterator plainIt(":/icons/toolbarButtons/plain");
    while(plainIt.hasNext()) {
        const auto path = plainIt.next();
        const auto fileName = plainIt.fileName();
        generate(path, uncheckedBase, dir + "/" + fileName + ".png");
    }
}

void IconLoader::generateAll(const int minWidgetDim, const int buttonDim) {
    const QDir eDir(eSettings::sSettingsDir());
    QDir iconsDir(eSettings::sSettingsDir() + "/icons");
#ifdef QT_DEBUG
    iconsDir.removeRecursively();
#endif
    const QString mkPath = "icons/" + QString::number(minWidgetDim);
    if(!eDir.mkpath(mkPath)) RuntimeThrow("Failed to mkpath '" + mkPath + "'");

    QDirIterator noInterIt(":/icons/noInterpolation");
    while(noInterIt.hasNext()) {
        const auto path = noInterIt.next();
        const auto fileName = noInterIt.fileName();
        const QImage img(path);
        int targetWidth = qCeil(img.width()*minWidgetDim/22.);
        if(qAbs(targetWidth - img.width()) % 2 == 1) targetWidth--;
        const auto scaled = img.scaledToWidth(targetWidth);
        scaled.save(iconsDir.filePath(QString::number(minWidgetDim) + "/" + fileName));
    }

    QDirIterator baseIt(":/icons");
    while(baseIt.hasNext()) {
        const auto path = baseIt.next();
        if(baseIt.fileInfo().isDir()) continue;
        const auto fileName = baseIt.fileName();
        const auto pngPath = iconsDir.filePath(QString::number(minWidgetDim) + "/" + fileName + ".png");
        generate(path, minWidgetDim/22., pngPath);
    }
    generateButtons(minWidgetDim, buttonDim);
}
