#include "gradientwidget.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include "Colors/helpers.h"
#include "mainwindow.h"
#include "Gradients/displayedgradientswidget.h"

GradientWidget::GradientWidget(QWidget *parent, MainWindow *mainWindow) :
    QWidget(parent) {
    setFixedHeight((3 + mNumberVisibleGradients + 0.5)*MIN_WIDGET_HEIGHT);
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

    connect(mMainWindow, SIGNAL(updateAll()),
            this, SLOT(updateAll()));
//    newGradient();
//    newGradient(Color(1.f, 1.f, 0.f), Color(0.f, 1.f, 1.f, 0.5f));
//    newGradient(Color(1.f, 0.f, 0.f), Color(0.f, 1.f, 0.f));
}

GradientWidget::~GradientWidget() {
    mGradients.clear();
}

void GradientWidget::updateAll() {
    mCurrentGradientWidget->update();
    mGradientsListWidget->getDisplayedGradientsWidget()->update();
    update();
}

void GradientWidget::setCurrentColorId(int id) {
    mCurrentColorId = id;
    //Color col = mCurrentGradient->getCurrentColorAt(mCurrentColorId);
    emit selectedColorChanged(
                mCurrentGradient->getColorAnimatorAt(mCurrentColorId));
    update();
}

void GradientWidget::updateNumberOfGradients() {
    mGradientsListWidget->setNumberGradients(mGradients.count());
}

void GradientWidget::addGradientToList(Gradient *gradient) {
    mGradients << gradient->ref<Gradient>();
    updateNumberOfGradients();
}

void GradientWidget::newGradient(const Color &color1,
                                 const Color &color2) {
    Gradient *newGradient = new Gradient(color1, color2);
    addGradientToList(newGradient);
    setCurrentGradient(newGradient);
    updateAll();
}

void GradientWidget::newGradient(const int &fromGradientId) {
    Gradient *fromGradient = mGradients.at(fromGradientId).data();
    Gradient *newGradient = (Gradient*)fromGradient->makeDuplicate();
    addGradientToList(newGradient);
    setCurrentGradient(mGradients.last().data());
    updateAll();
}

void GradientWidget::clearAll() {
    mGradients.clear();

    mCurrentGradient = NULL;
    mCenterGradientId = 1;
    mCurrentColorId = 0;
    updateAll();
}

int GradientWidget::getGradientIndex(Gradient *child) {
    int index = -1;
    for(int i = 0; i < mGradients.count(); i++) {
        if(mGradients.at(i) == child) {
            index = i;
        }
    }
    return index;
}

void GradientWidget::removeGradientFromList(Gradient *toRemove) {
    mGradients.removeAt(getGradientIndex(toRemove));
    if(mCurrentGradient == toRemove) {
        mCurrentGradient = NULL;
        setCurrentGradient((Gradient*) NULL);
    }
    updateNumberOfGradients();
}

void GradientWidget::removeGradient(int gradientId)
{
    Gradient *toRemove = mGradients.at(gradientId).data();
    if(toRemove->affectsPaths()) {
        return;
    }
    removeGradientFromList(toRemove);

    mCenterGradientId = clampInt(mCenterGradientId, 1,
                                 mGradients.length() - 2);
    updateAll();
}

void GradientWidget::setCurrentColor(GLfloat h,
                                     GLfloat s,
                                     GLfloat v,
                                     GLfloat a) {
    Color newColor;
    newColor.setHSV(h, s, v, a);
    mCurrentGradient->replaceColor(mCurrentColorId, newColor);
    updateAll();
}

void GradientWidget::setCurrentGradient(Gradient *gradient,
                                        const bool &emitChange) {
    if(mCurrentGradient != NULL) {
        disconnect(mCurrentGradient,
                   SIGNAL(resetGradientWidgetColorIdIfEquals(Gradient*,int)),
                   this,
                   SLOT(resetColorIdIfEquals(Gradient*,int)));
    }
    if(gradient == NULL) {
        if(mGradients.isEmpty()) newGradient();
        setCurrentGradient(0);
        return;
    } else if(mCurrentGradient == gradient) {
        return;
    }
    mCurrentGradient = gradient;
    if(mCurrentGradient != NULL) {
        connect(mCurrentGradient,
                SIGNAL(resetGradientWidgetColorIdIfEquals(Gradient*,int)),
                this,
                SLOT(resetColorIdIfEquals(Gradient*,int)));
    }
    setCurrentColorId(0);

    if(emitChange) {
        emit currentGradientChanged(mCurrentGradient);
    }
}

Gradient *GradientWidget::getCurrentGradient() {
    return mCurrentGradient;
}

Color GradientWidget::getCurrentColor() {
    if(mCurrentGradient == NULL) return Color();
    return mCurrentGradient->getCurrentColorAt(mCurrentColorId);
}

ColorAnimator *GradientWidget::getCurrentColorAnimator() {
    if(mCurrentGradient == NULL) return NULL;
    return mCurrentGradient->getColorAnimatorAt(mCurrentColorId);
}

void GradientWidget::resetColorIdIfEquals(Gradient *gradient, const int &id) {
    if(gradient == mCurrentGradient) {
        if(id == mCurrentColorId) {
            mCurrentColorId = 0;
        }
    }
}

void GradientWidget::setCurrentGradient(const int &listId) {
    if(listId >= mGradients.length()) return;
    setCurrentGradient(mGradients.at(listId).data());
}

void GradientWidget::colorRightPress(const int &x,
                                     const QPoint &point) {
    colorLeftPress(x);
    if(mCurrentGradient != NULL) {
        QMenu menu(this);
        menu.addAction("Delete Color");
        menu.addAction("Add Color");
        QAction *selected_action = menu.exec(point);
        if(selected_action != NULL) {
            if(selected_action->text() == "Delete Color") {

                if(mCurrentGradient->getColorCount() < 2) {
                    mCurrentGradient->replaceColor(mCurrentColorId,
                                                   Color(0.f, 0.f, 0.f, 1.f));
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
                mCurrentGradient->addColor(Color(0.f, 0.f, 0.f, 1.f));
                emit gradientSettingsChanged();
                finishGradientTransform();
                updateAll();
            }
        }
        else {

        }
    }
}

int GradientWidget::getColorIdAtX(const int &x) {
    int nCols = mCurrentGradient->getColorCount();
    return clampInt(x*nCols/width(), 0, nCols - 1);
}

void GradientWidget::colorLeftPress(const int &x) {
    if(mCurrentGradient != NULL) {
        setCurrentColorId(getColorIdAtX(x));
    }
    updateAll();
}

void GradientWidget::moveColor(const int &x) {
    if(mCurrentGradient != NULL) {
        int nCols = mCurrentGradient->getColorCount();
            int colorId = clampInt(x*nCols/width(), 0, nCols - 1);
            if(colorId != mCurrentColorId) {
                startGradientTransform();
                mCurrentGradient->swapColors(mCurrentColorId, colorId);
                setCurrentColorId(colorId);
                emit gradientSettingsChanged();
                finishGradientTransform();
                updateAll();
            }
    }
}

void GradientWidget::drawCurrentGradientColors(const int &x,
                                               const int &y,
                                               const int &width,
                                               const int &height) {
    if(mCurrentGradient == NULL) return;

    int len = mCurrentGradient->getColorCount();
    Color nextColor = mCurrentGradient->getCurrentColorAt(0);
    GLfloat cX = x;
    GLfloat segWidth = width/(GLfloat)len;
    for(int i = 0; i < len; i++) {
        Color color = nextColor;
        GLWidget::drawRect(cX, y, segWidth, height,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 false, false, false, false);
        if(i == mCurrentColorId) {
            GLWidget::drawBorder(cX, y, segWidth, height);
        }
        if(i + 1 == len) break;
        nextColor = mCurrentGradient->getCurrentColorAt(i + 1);
        cX += segWidth;
    }
}

void GradientWidget::drawCurrentGradient(const int &x,
                                         const int &y,
                                         const int &width,
                                         const int &height) {
    if(mCurrentGradient == NULL) return;
    GLWidget::drawGradient(mCurrentGradient,
                 x, y, width, height);
}

void GradientWidget::drawGradients(const int &displayedTop,
                                   const int &topGradientId,
                                   const int &numberVisibleGradients,
                                   const int &scrollItemHeight) {
    int gradListHeight = scrollItemHeight*numberVisibleGradients;
    GLWidget::drawMeshBg(0, displayedTop,
               width(), gradListHeight);

    int yT = displayedTop;
    for(int i = topGradientId; i < mGradients.count(); i++) {
        Gradient *gradient = mGradients.at(i).data();
        GLWidget::drawGradient(gradient,
                     0, yT,
                     width(), scrollItemHeight);
        if(gradient == mCurrentGradient) {
            GLWidget::drawBorder(0, yT,
                       width(), scrollItemHeight);
        }
        yT += scrollItemHeight;
    }
}

void GradientWidget::drawHoveredGradientBorder(const int &displayedTop,
                                               const int &topGradientId,
                                               const int &hoveredGradId,
                                               const int &scrollItemHeight) {
    int gradId = topGradientId + hoveredGradId;
    if(gradId >= mGradients.count() || gradId < 0) return;
    GLWidget::drawHoverBorder(0,
                              displayedTop + hoveredGradId*scrollItemHeight,
                              width(), scrollItemHeight);
}

void GradientWidget::drawHoveredColorBorder(const int &hoveredX,
                                            const int &colHeight) {
    if(hoveredX < 0 || hoveredX > width() || mCurrentGradient == NULL) return;
    int colId = getColorIdAtX(hoveredX);
    int len = mCurrentGradient->getColorCount();
    int colWidth = width()/len;
    GLWidget::drawHoverBorder(colId*colWidth, 0,
                              colWidth, colHeight);
}

void GradientWidget::updateAfterFrameChanged(const int &absFrame) {
    Q_FOREACH(const QSharedPointer<Gradient> &gradient, mGradients) {
        gradient->prp_setAbsFrame(absFrame);
    }
}

void GradientWidget::clearGradientsLoadIds() {
    foreach(const QSharedPointer<Gradient> &gradient, mGradients) {
        gradient->setLoadId(-1);
    }
}

void GradientWidget::setGradientLoadIds() {
    int id = 0;
    foreach(const QSharedPointer<Gradient> &gradient, mGradients) {
        gradient->setLoadId(id);
        id++;
    }
}

void GradientWidget::gradientLeftPressed(const int &gradId) {
    if(gradId >= mGradients.count() || gradId < 0) return;
    setCurrentGradient(gradId);
    MainWindow::getInstance()->callUpdateSchedulers();
}

void GradientWidget::gradientContextMenuReq(const int &gradId,
                                            const QPoint globalPos) {
    bool gradPressed = gradId < mGradients.count() && gradId >= 0;
    QMenu menu(this);
    menu.addAction("New Gradient");
    if(gradPressed) {
        menu.addAction("Duplicate Gradient");
        menu.addAction("Delete Gradient");
    }
    QAction *selected_action = menu.exec(globalPos);
    if(selected_action != NULL) {
        if(selected_action->text() == "Delete Gradient") {
            removeGradient(gradId);
        } else if(selected_action->text() == "Duplicate Gradient") {
            newGradient(gradId);
        } else if(selected_action->text() == "New Gradient") {
            newGradient();
        }
    } else {

    }
}

void GradientWidget::startSelectedColorTransform() {
    if(mCurrentGradient == NULL) return;
    mCurrentGradient->startColorIdTransform(mCurrentColorId);
}

void GradientWidget::finishGradientTransform()
{
    if(mCurrentGradient == NULL) return;
    mCurrentGradient->prp_finishTransform();
}

void GradientWidget::startGradientTransform()
{
    if(mCurrentGradient == NULL) return;
    mCurrentGradient->prp_startTransform();
}
