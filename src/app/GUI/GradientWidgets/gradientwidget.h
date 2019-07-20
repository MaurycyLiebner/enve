#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H
#include "GUI/fillstrokesettings.h"
#include <QComboBox>
#include <QScrollArea>
#include "GUI/GradientWidgets/gradientslistwidget.h"
#include "GUI/GradientWidgets/currentgradientwidget.h"

class GradientWidget : public QWidget {
    Q_OBJECT
public:
    GradientWidget(QWidget * const parent);

    void setCurrentGradient(Gradient *gradient,
                            const bool emitChange = true);
    Gradient *getCurrentGradient();
    QColor getColor();
    ColorAnimator *getColorAnimator();

    void moveColor(const int x);

    void finishGradientTransform();
    void startGradientTransform();
    void clearAll();
    void startSelectedColorTransform();
    void updateNumberOfGradients();

    void gradientLeftPressed(const int gradId);
    void gradientContextMenuReq(const int gradId,
                                const QPoint globalPos);

    void colorRightPress(const int x, const QPoint &point);
    void colorLeftPress(const int x);
    int getColorIdAtX(const int x);

    void updateAfterFrameChanged(const int absFrame);

    int getGradientsCount() const {
        return Document::sInstance->fGradients.count();
    }

    Gradient* getGradientAt(const int id) const {
        return Document::sInstance->fGradients.at(id).get();
    }

    int getColorId() const {
        return mCurrentColorId;
    }

    void setCurrentColor(const QColor &col);
signals:
    void selectedColorChanged(ColorAnimator*);
    void currentGradientChanged(Gradient *gradient);
private:
    void updateAll();

    QVBoxLayout *mMainLayout;
    GradientsListWidget *mGradientsListWidget;
    CurrentGradientWidget *mCurrentGradientWidget;

    int mNumberVisibleGradients = 6;
    int mHalfHeight = 64;
    int mQuorterHeight = 32;
    int mScrollItemHeight;

    Gradient *mCurrentGradient = nullptr;
    ColorAnimator *mCurrentColor = nullptr;
    int mCurrentColorId = 0;
    int mCenterGradientId = 1;
    void setCurrentGradient(const int listId);
    void setCurrentColorId(const int id);
};

#endif // GRADIENTWIDGET_H
