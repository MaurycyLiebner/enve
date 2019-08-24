#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QString>

namespace IconLoader {
    void generateAll(const int minWidgetDim, const int buttonDim);
    bool generate(const QString& svgPath,
                  const qreal scale,
                  const QString& pngPath);
};

#endif // ICONLOADER_H
