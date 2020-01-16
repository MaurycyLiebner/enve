// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

    void clearAll();
    void updateNumberOfGradients();

    void gradientLeftPressed(const int gradId);
    void gradientContextMenuReq(const int gradId,
                                const QPoint globalPos);

    void colorRightPress(const int x, const QPoint &point);
    void colorLeftPress(const int x);
    void colorRelease();
    int getColorIdAtX(const int x);

    void updateAfterFrameChanged(const int absFrame);

    int getGradientsCount() const;

    Gradient* getGradientAt(const int id) const;

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

    bool mReordering = false;
    bool mFirstMove = false;

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
