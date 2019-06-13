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
    setFixedHeight(qRound((3 + mNumberVisibleGradients + 0.5)*MIN_WIDGET_HEIGHT));
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(0);
    mMainLayout->setSpacing(0);
    mMainLayout->setAlignment(Qt::AlignTop);
    mGradientsListWidget = new GradientsListWidget(this, this);
    mCurrentGradientWidget = new CurrentGradientWidget(this, this);
    mMainLayout->addWidget(mGradientsListWidget);
    mMainLayout->addSpacing(MIN_WIDGET_HEIGHT/2);
    mMainLayout->addWidget(mCurrentGradientWidget);
    setLayout(mMainLayout);

    mScrollItemHeight = MIN_WIDGET_HEIGHT;

    mMainWindow = mainWindow;  

    connect(mMainWindow, &MainWindow::updateAll,
            this, &GradientWidget::updateAll);
//    newGradient();
//    newGradient(Color(1.f, 1.f, 0.f), Color(0.f, 1.f, 1.f, 0.5f));
//    newGradient(Color(1.f, 0.f, 0.f), Color(0.f, 1.f, 0.f));
}

GradientWidget::~GradientWidget() {}

void GradientWidget::updateAll() {
    mCurrentGradientWidget->update();
    mGradientsListWidget->getDisplayedGradientsWidget()->update();
    update();
}

void GradientWidget::setCurrentColorId(const int id) {
    mCurrentColorId = id;
    //Color col = mCurrentGradient->getColorAt(mCurrentColorId);
    const auto currCol = mCurrentGradient->getColorAnimatorAt(mCurrentColorId);
    emit selectedColorChanged(currCol);
    update();
}

void GradientWidget::updateNumberOfGradients() {
    mGradientsListWidget->setNumberGradients(mGradients.count());
}

void GradientWidget::addGradientToList(const qsptr<Gradient>& gradient) {
    mGradients << gradient;
    updateNumberOfGradients();
}

void GradientWidget::newGradient(const QColor &color1,
                                 const QColor &color2) {
    auto newGradient = SPtrCreate(Gradient)(color1, color2);
    addGradientToList(newGradient);
    setCurrentGradient(newGradient.get());
    updateAll();
}

void GradientWidget::newGradient(const int fromGradientId) {
    const Gradient * const fromGradient = mGradients.at(fromGradientId).data();
    auto newGradient = SPtrCreate(Gradient)();
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    fromGradient->writeProperty(&buffer);
    if(buffer.reset()) newGradient->readProperty(&buffer);
    else return;
    buffer.close();
    addGradientToList(newGradient);
    setCurrentGradient(mGradients.last().data());
    updateAll();
}

void GradientWidget::clearAll() {
    mGradients.clear();

    mCurrentGradient = nullptr;
    mCenterGradientId = 1;
    mCurrentColorId = 0;
    updateAll();
}

int GradientWidget::getGradientIndex(Gradient *child) {
    int index = -1;
    for(int i = 0; i < mGradients.count(); i++) {
        if(mGradients.at(i) == child) index = i;
    }
    return index;
}

void GradientWidget::removeGradientFromList(const qsptr<Gradient> &toRemove) {
    mGradients.removeAt(getGradientIndex(toRemove.get()));
    if(mCurrentGradient == toRemove) {
        mCurrentGradient = nullptr;
        setCurrentGradient(nullptr);
    }
    updateNumberOfGradients();
}

void GradientWidget::removeGradient(const int gradientId) {
    qsptr<Gradient> toRemove = mGradients.at(gradientId);
    if(toRemove->affectsPaths()) return;
    removeGradientFromList(toRemove);

    mCenterGradientId = clampInt(mCenterGradientId, 1,
                                 mGradients.length() - 2);
    updateAll();
}

void GradientWidget::setCurrentColor(const QColor& col) {
    mCurrentGradient->replaceColor(mCurrentColorId, col);
    updateAll();
}

void GradientWidget::setCurrentGradient(Gradient *gradient,
                                        const bool emitChange) {
    if(mCurrentGradient) {
        disconnect(mCurrentGradient,
                   &Gradient::resetGradientWidgetColorIdIfEquals,
                   this, &GradientWidget::resetColorIdIfEquals);
    }
    if(!gradient) {
        if(mGradients.isEmpty()) newGradient();
        setCurrentGradient(0);
        return;
    } else if(mCurrentGradient == gradient) {
        return;
    }
    mCurrentGradient = gradient;
    if(mCurrentGradient) {
        connect(mCurrentGradient,
                &Gradient::resetGradientWidgetColorIdIfEquals,
                this, &GradientWidget::resetColorIdIfEquals);
    }
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

void GradientWidget::resetColorIdIfEquals(Gradient *gradient, const int id) {
    if(gradient == mCurrentGradient) {
        if(id == mCurrentColorId) mCurrentColorId = 0;
    }
}

void GradientWidget::setCurrentGradient(const int listId) {
    if(listId >= mGradients.length()) return;
    setCurrentGradient(mGradients.at(listId).data());
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
            mMainWindow->queScheduledTasksAndUpdate();
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
            mMainWindow->queScheduledTasksAndUpdate();
        }
    }
}

void GradientWidget::updateAfterFrameChanged(const int absFrame) {
    for(const auto& gradient : mGradients)
        gradient->anim_setAbsFrame(absFrame);
}

void GradientWidget::clearGradientsLoadIds() {
    for(const auto& gradient : mGradients)
        gradient->setLoadId(-1);
}

void GradientWidget::setGradientLoadIds() {
    int id = 0;
    for(const auto& gradient : mGradients) {
        gradient->setLoadId(id);
        id++;
    }
}

void GradientWidget::gradientLeftPressed(const int gradId) {
    if(gradId >= mGradients.count() || gradId < 0) return;
    setCurrentGradient(gradId);
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void GradientWidget::gradientContextMenuReq(const int gradId,
                                            const QPoint globalPos) {
    const bool gradPressed = gradId < mGradients.count() && gradId >= 0;
    QMenu menu(this);
    menu.addAction("New Gradient");
    if(gradPressed) {
        menu.addAction("Duplicate Gradient");
        menu.addAction("Delete Gradient");
    }
    const auto selected_action = menu.exec(globalPos);
    if(selected_action) {
        if(selected_action->text() == "Delete Gradient") {
            removeGradient(gradId);
        } else if(selected_action->text() == "Duplicate Gradient") {
            newGradient(gradId);
        } else if(selected_action->text() == "New Gradient") {
            newGradient();
        }
        mMainWindow->queScheduledTasksAndUpdate();
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
