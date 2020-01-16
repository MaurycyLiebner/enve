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

#include "gradientwidget.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include "colorhelpers.h"
#include "GUI/mainwindow.h"
#include "GUI/GradientWidgets/displayedgradientswidget.h"
#include "Animators/gradient.h"

GradientWidget::GradientWidget(QWidget * const parent) :
    QWidget(parent) {
    setFixedHeight(qRound((3 + mNumberVisibleGradients + 0.5)*MIN_WIDGET_DIM));
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(0);
    mMainLayout->setSpacing(0);
    mMainLayout->setAlignment(Qt::AlignTop);
    mGradientsListWidget = new GradientsListWidget(this, this);
    mCurrentGradientWidget = new CurrentGradientWidget(this, this);
    mMainLayout->addWidget(mGradientsListWidget);
    mMainLayout->addSpacing(MIN_WIDGET_DIM/2);
    mMainLayout->addWidget(mCurrentGradientWidget);
    setLayout(mMainLayout);

    mScrollItemHeight = MIN_WIDGET_DIM;


    connect(Document::sInstance, &Document::gradientCreated,
            this, [this](Gradient* const gradient) {
        connect(gradient, &Gradient::prp_absFrameRangeChanged,
                this, &GradientWidget::updateAll);
        updateNumberOfGradients();
    });
    connect(Document::sInstance, &Document::gradientRemoved,
            this, [this](Gradient* const gradient) {
        disconnect(gradient, nullptr, this, nullptr);
        updateNumberOfGradients();
    });
//    newGradient();
//    newGradient(Color(1.f, 1.f, 0.f), Color(0.f, 1.f, 1.f, 0.5f));
//    newGradient(Color(1.f, 0.f, 0.f), Color(0.f, 1.f, 0.f));
}

void GradientWidget::updateAll() {
    mCurrentGradientWidget->update();
    mGradientsListWidget->getDisplayedGradientsWidget()->update();
    update();
}

void GradientWidget::setCurrentColorId(const int id) {
    if(mCurrentColor) disconnect(mCurrentColor, nullptr, this, nullptr);
    mCurrentColorId = id;
    if(!mCurrentGradient) {
        mCurrentColor = nullptr;
        return;
    }
    mCurrentColor = mCurrentGradient->getChild(mCurrentColorId);
    connect(mCurrentColor, &QObject::destroyed, this, [this]() {
        setCurrentColorId(0);
    });
    emit selectedColorChanged(mCurrentColor);
    update();
}

void GradientWidget::updateNumberOfGradients() {
    mGradientsListWidget->setNumberGradients(
                Document::sInstance->fGradients.count());
}

void GradientWidget::clearAll() {
    mCurrentGradient = nullptr;
    mCenterGradientId = 1;
    mCurrentColorId = 0;
    updateAll();
}

void GradientWidget::setCurrentColor(const QColor& col) {
    mCurrentGradient->replaceColor(mCurrentColorId, col);
    updateAll();
}

void GradientWidget::setCurrentGradient(Gradient *gradient,
                                        const bool emitChange) {
    if(!gradient) {
        if(Document::sInstance->fGradients.isEmpty()) {
            const auto newGrad = Document::sInstance->createNewGradient();
            newGrad->addColor(Qt::black);
            newGrad->addColor(Qt::white);
        }
        setCurrentGradient(0);
        return;
    } else if(mCurrentGradient == gradient) {
        return;
    }
    mCurrentGradient = gradient;
    setCurrentColorId(0);

    if(emitChange) emit currentGradientChanged(mCurrentGradient);
}

Gradient *GradientWidget::getCurrentGradient() {
    return mCurrentGradient;
}

QColor GradientWidget::getColor() {
    if(!mCurrentGradient) return QColor();
    return mCurrentGradient->getColorAt(mCurrentColorId);
}

ColorAnimator *GradientWidget::getColorAnimator() {
    if(!mCurrentGradient) return nullptr;
    return mCurrentGradient->getChild(mCurrentColorId);
}

void GradientWidget::setCurrentGradient(const int listId) {
    if(listId >= Document::sInstance->fGradients.length()) return;
    setCurrentGradient(Document::sInstance->fGradients.at(listId).data());
}

void GradientWidget::colorRightPress(const int x, const QPoint &point) {
    colorLeftPress(x);
    if(mCurrentGradient) {
        QMenu menu(this);
        menu.addAction("Delete Color");
        menu.addAction("Add Color");
        const auto selected_action = menu.exec(point);
        if(selected_action) {
            if(selected_action->text() == "Delete Color") {
                if(mCurrentGradient->ca_getNumberOfChildren() < 2) {
                    mCurrentGradient->replaceColor(mCurrentColorId,
                                                   QColor(0, 0, 0));
                } else {
                    mCurrentGradient->takeChildAt(mCurrentColorId);
                }
                startGradientTransform();
                setCurrentColorId(0);
                finishGradientTransform();
                updateAll();
            } else if(selected_action->text() == "Add Color") {
                startGradientTransform();
                mCurrentGradient->addColor(QColor(0, 0, 0));
                finishGradientTransform();
                updateAll();
            }
            Document::sInstance->actionFinished();
        } else {

        }
    }
}

int GradientWidget::getColorIdAtX(const int x) {
    const int nCols = mCurrentGradient->ca_getNumberOfChildren();
    return clampInt(x*nCols/width(), 0, nCols - 1);
}

void GradientWidget::colorLeftPress(const int x) {
    if(mCurrentGradient) setCurrentColorId(getColorIdAtX(x));
    updateAll();
}

void GradientWidget::moveColor(const int x) {
    if(mCurrentGradient) {
        const int nCols = mCurrentGradient->ca_getNumberOfChildren();
        const int colorId = clampInt(x*nCols/width(), 0, nCols - 1);
        if(colorId != mCurrentColorId) {
            startGradientTransform();
            mCurrentGradient->ca_swapChildren(mCurrentColorId, colorId);
            setCurrentColorId(colorId);
            finishGradientTransform();
            updateAll();
            Document::sInstance->actionFinished();
        }
    }
}

void GradientWidget::updateAfterFrameChanged(const int absFrame) {
    for(const auto& gradient : Document::sInstance->fGradients)
        gradient->anim_setAbsFrame(absFrame);
}

int GradientWidget::getGradientsCount() const {
    return Document::sInstance->fGradients.count();
}

Gradient *GradientWidget::getGradientAt(const int id) const {
    return Document::sInstance->fGradients.at(id).get();
}

void GradientWidget::gradientLeftPressed(const int gradId) {
    if(gradId >= Document::sInstance->fGradients.count() || gradId < 0) return;
    setCurrentGradient(gradId);
    Document::sInstance->actionFinished();
}

void GradientWidget::gradientContextMenuReq(const int gradId,
                                            const QPoint globalPos) {
    const bool gradPressed = gradId < Document::sInstance->fGradients.count() && gradId >= 0;
    QMenu menu(this);
    menu.addAction("New Gradient");
    if(gradPressed) {
        menu.addAction("Duplicate Gradient");
        menu.addAction("Delete Gradient");
    }
    const auto selected_action = menu.exec(globalPos);
    if(selected_action) {
        if(selected_action->text() == "Delete Gradient") {
            Document::sInstance->removeGradient(gradId);
        } else if(selected_action->text() == "Duplicate Gradient") {
            Document::sInstance->duplicateGradient(gradId);
        } else if(selected_action->text() == "New Gradient") {
            const auto newGrad = Document::sInstance->createNewGradient();
            newGrad->addColor(Qt::black);
            newGrad->addColor(Qt::white);
            setCurrentGradient(newGrad);
        }
        Document::sInstance->actionFinished();
    } else {

    }
}

void GradientWidget::startSelectedColorTransform() {
    if(!mCurrentGradient) return;
    mCurrentGradient->startColorIdTransform(mCurrentColorId);
}

void GradientWidget::finishGradientTransform() {
    if(!mCurrentGradient) return;
    mCurrentGradient->prp_finishTransform();
}

void GradientWidget::startGradientTransform() {
    if(!mCurrentGradient) return;
    mCurrentGradient->prp_startTransform();
}
