#include "gradientwidget.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include "colorhelpers.h"
#include "GUI/mainwindow.h"
#include "GUI/GradientWidgets/displayedgradientswidget.h"
#include "Animators/gradient.h"

GradientWidget::GradientWidget(QWidget *parent, MainWindow *mainWindow) :
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

    mMainWindow = mainWindow;  

    connect(mMainWindow, &MainWindow::updateAll,
            this, &GradientWidget::updateAll);

    connect(Document::sInstance, &Document::gradientCreated,
            this, &GradientWidget::updateNumberOfGradients);
    connect(Document::sInstance, qOverload<int>(&Document::gradientRemoved),
            this, &GradientWidget::updateNumberOfGradients);
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
    mCurrentColor = mCurrentGradient->getColorAnimatorAt(mCurrentColorId);
    connect(mCurrentColor, &QObject::destroyed, this, [this]() {
        setCurrentColorId(0);
    });
    emit selectedColorChanged(mCurrentColor);
    update();
}

void GradientWidget::updateNumberOfGradients() {
    mGradientsListWidget->setNumberGradients(Document::sInstance->fGradients.count());
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
    return mCurrentGradient->getColorAnimatorAt(mCurrentColorId);
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
                if(mCurrentGradient->getColorCount() < 2) {
                    mCurrentGradient->replaceColor(mCurrentColorId,
                                                   QColor(0, 0, 0));
                } else {
                    mCurrentGradient->removeColor(mCurrentColorId);
                }
                startGradientTransform();
                setCurrentColorId(0);
                emit gradientSettingsChanged();
                finishGradientTransform();
                updateAll();
            } else if(selected_action->text() == "Add Color") {
                startGradientTransform();
                mCurrentGradient->addColor(QColor(0, 0, 0));
                emit gradientSettingsChanged();
                finishGradientTransform();
                updateAll();
            }
            mMainWindow->queTasksAndUpdate();
        } else {

        }
    }
}

int GradientWidget::getColorIdAtX(const int x) {
    const int nCols = mCurrentGradient->getColorCount();
    return clampInt(x*nCols/width(), 0, nCols - 1);
}

void GradientWidget::colorLeftPress(const int x) {
    if(mCurrentGradient) setCurrentColorId(getColorIdAtX(x));
    updateAll();
}

void GradientWidget::moveColor(const int x) {
    if(mCurrentGradient) {
        const int nCols = mCurrentGradient->getColorCount();
        const int colorId = clampInt(x*nCols/width(), 0, nCols - 1);
        if(colorId != mCurrentColorId) {
            startGradientTransform();
            mCurrentGradient->swapColors(mCurrentColorId, colorId);
            setCurrentColorId(colorId);
            emit gradientSettingsChanged();
            finishGradientTransform();
            updateAll();
            mMainWindow->queTasksAndUpdate();
        }
    }
}

void GradientWidget::updateAfterFrameChanged(const int absFrame) {
    for(const auto& gradient : Document::sInstance->fGradients)
        gradient->anim_setAbsFrame(absFrame);
}

void GradientWidget::gradientLeftPressed(const int gradId) {
    if(gradId >= Document::sInstance->fGradients.count() || gradId < 0) return;
    setCurrentGradient(gradId);
    MainWindow::getInstance()->queTasksAndUpdate();
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
        mMainWindow->queTasksAndUpdate();
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
