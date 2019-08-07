#ifndef GRADIENT_H
#define GRADIENT_H
#include <QGradientStops>
#include "Animators/complexanimator.h"
class ColorAnimator;
class PathBox;

enum class GradientType : short { LINEAR, RADIAL };

class Gradient : public ComplexAnimator {
    Q_OBJECT
    e_OBJECT
protected:
    Gradient();
    Gradient(const QColor &color1, const QColor &color2);
public:
    bool SWT_isGradient() const { return true; }
    void prp_startTransform();
    void prp_setParentFrameShift(const int shift,
                                 ComplexAnimator* parentAnimator = nullptr) {
        Q_UNUSED(shift);
        if(!parentAnimator) return;
        for(const auto &key : anim_mKeys) {
            parentAnimator->ca_updateDescendatKeyFrame(key);
        }
    }

    int prp_getFrameShift() const {
        return 0;
    }

    int prp_getParentFrameShift() const {
        return 0;
    }

    void write(const int id, QIODevice * const dst);
    int read(QIODevice * const src);
    int getReadWriteId();
    void clearReadWriteId();

    void swapColors(const int id1, const int id2);
    void removeColor(const qsptr<ColorAnimator> &color);
    void addColor(const QColor &color);
    void replaceColor(const int id, const QColor &color);
    void addPath(PathBox * const path);
    void removePath(PathBox * const path);
    bool affectsPaths();

    void updateQGradientStops(const UpdateReason reason);

    void addColorToList(const QColor &color);
    QColor getColorAt(const int id);
    int getColorCount();

    QColor getLastQGradientStopQColor();
    QColor getFirstQGradientStopQColor();

    QGradientStops getQGradientStops();
    void startColorIdTransform(const int id);
    void addColorToList(const qsptr<ColorAnimator> &newColorAnimator);
    ColorAnimator *getColorAnimatorAt(const int id);
    void removeColor(const int id);

    bool isEmpty() const;

    QGradientStops getQGradientStopsAtAbsFrame(const qreal absFrame);
private:
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);

    int mReadWriteId = -1;
    QGradientStops mQGradientStops;
    QList<qsptr<ColorAnimator>> mColors;
    QList<qptr<PathBox>> mAffectedPaths;
    qptr<ColorAnimator> mCurrentColor;
};

#endif // GRADIENT_H
