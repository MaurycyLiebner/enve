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

#include "displayedgradientswidget.h"
#include <QPainter>
#include "GUI/GradientWidgets/gradientwidget.h"
#include "Animators/gradient.h"
#include "Private/document.h"
#include "canvas.h"

DisplayedGradientsWidget::DisplayedGradientsWidget(QWidget *parent) :
    GLWidget(parent) {
    setMouseTracking(true);
    connect(Document::sInstance, &Document::activeSceneSet,
            this, &DisplayedGradientsWidget::setScene);
}

void DisplayedGradientsWidget::setScene(Canvas * const scene) {
    if(scene == mScene) return;
    mGradients.clear();
    setSelectedGradient(nullptr);
    auto& conn = mScene.assign(scene);
    if(scene) {
        conn << connect(scene, &Canvas::gradientCreated,
                this, [this](SceneBoundGradient* const gradient) {
            addGradient(gradient);
        });
        conn << connect(scene, &Canvas::gradientRemoved,
                this, [this](SceneBoundGradient* const gradient) {
            removeGradient(gradient);
        });
        for(const auto& gradient : scene->gradients()) {
            addGradient(gradient.get());
        }
    }
    updateHeight();
}

void DisplayedGradientsWidget::incTop(const int inc) {
    mDisplayedTop -= inc;
    updateTopGradientId();
}

void DisplayedGradientsWidget::setTop(const int top) {
    mDisplayedTop = top;
    updateTopGradientId();
}

void DisplayedGradientsWidget::updateTopGradientId() {
    const int newGradientId = mDisplayedTop/MIN_WIDGET_DIM;
    mHoveredGradientId += newGradientId - mTopGradientId;
    mTopGradientId = newGradientId;
    update();
}

void DisplayedGradientsWidget::updateHeight() {
    setFixedHeight(qMax(minimumHeight(), (mGradients.count() + 1)*MIN_WIDGET_DIM));
}

#include "GUI/ColorWidgets/colorwidgetshaders.h"
void DisplayedGradientsWidget::paintGL() {
    const int nVisible = qMin(mGradients.count() - mTopGradientId,
                              mMaxVisibleGradients);
    int gradY = mDisplayedTop;
    glClearColor(0.3f, 0.3f, 0.3f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(GRADIENT_PROGRAM.fID);
    glBindVertexArray(mPlainSquareVAO);
    for(int i = mTopGradientId; i < mTopGradientId + nVisible; i++) {
        const int yInverted = height() - gradY - MIN_WIDGET_DIM;
        const auto gradient = mGradients.at(i);
        const int nColors = gradient->ca_getNumberOfChildren();
        QColor lastColor = gradient->getColorAt(0);
        int xT = 0;
        const float xInc = static_cast<float>(width())/(nColors - 1);
        glUniform2f(GRADIENT_PROGRAM.fMeshSizeLoc,
                    MIN_WIDGET_DIM/(3.f*xInc), 1.f/3);
        for(int j = 1; j < nColors; j++) {
            const QColor color = gradient->getColorAt(j);
            glViewport(xT, yInverted, qRound(xInc), MIN_WIDGET_DIM);

            glUniform4f(GRADIENT_PROGRAM.fRGBAColor1Loc,
                        lastColor.redF(), lastColor.greenF(),
                        lastColor.blueF(), lastColor.alphaF());
            glUniform4f(GRADIENT_PROGRAM.fRGBAColor2Loc,
                        color.redF(), color.greenF(),
                        color.blueF(), color.alphaF());

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            xT += qRound(xInc);
            lastColor = color;
        }
        glViewport(0, yInverted, width(), MIN_WIDGET_DIM);
        if(gradient == mSelectedGradient) {
            glUseProgram(DOUBLE_BORDER_PROGRAM.fID);
            glUniform2f(DOUBLE_BORDER_PROGRAM.fInnerBorderSizeLoc,
                        1.f/width(), 1.f/MIN_WIDGET_DIM);
            glUniform4f(DOUBLE_BORDER_PROGRAM.fInnerBorderColorLoc,
                        1.f, 1.f, 1.f, 1.f);
            glUniform2f(DOUBLE_BORDER_PROGRAM.fOuterBorderSizeLoc,
                        1.f/width(), 1.f/MIN_WIDGET_DIM);
            glUniform4f(DOUBLE_BORDER_PROGRAM.fOuterBorderColorLoc,
                        0.f, 0.f, 0.f, 1.f);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glUseProgram(GRADIENT_PROGRAM.fID);
        }
        if(i == mHoveredGradientId || i == mContextMenuGradientId) {
            glUseProgram(BORDER_PROGRAM.fID);
            glUniform2f(BORDER_PROGRAM.fBorderSizeLoc,
                        1.f/width(), 1.f/MIN_WIDGET_DIM);
            glUniform4f(BORDER_PROGRAM.fBorderColorLoc,
                        1.f, 1.f, 1.f, 1.f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glUseProgram(GRADIENT_PROGRAM.fID);
        }
        gradY += MIN_WIDGET_DIM;
    }
}

void DisplayedGradientsWidget::setSelectedGradient(SceneBoundGradient *gradient) {
    if(mSelectedGradient == gradient) return;
    mSelectedGradient = gradient;
    emit selectionChanged(mSelectedGradient);
    update();
}

void DisplayedGradientsWidget::addGradient(SceneBoundGradient * const gradient) {
    auto& conn = mGradients.addObj(gradient);
    conn << connect(gradient, &SceneBoundGradient::prp_absFrameRangeChanged,
                    this, qOverload<>(&QWidget::update));
    if(!mSelectedGradient) setSelectedGradient(gradient);
    updateHeight();
    update();
}

void DisplayedGradientsWidget::removeGradient(SceneBoundGradient * const gradient) {
    const int removeId = mGradients.indexOf(gradient);
    if(!mGradients.removeObj(gradient)) return;
    if(mSelectedGradient == gradient) {
        const int selectId = qMin(removeId, mGradients.count() - 1);
        setSelectedGradient(selectId >= 0 ? mGradients.at(selectId) : nullptr);
    }
    updateHeight();
    update();
}

void DisplayedGradientsWidget::gradientLeftPressed(const int gradId) {
    if(!mScene) return;
    if(gradId >= mScene->gradients().count() || gradId < 0) return;
    const auto gradient = mGradients.at(gradId);
    setSelectedGradient(gradient);
    emit triggered(gradient);
    Document::sInstance->actionFinished();
}

void DisplayedGradientsWidget::gradientContextMenuReq(
        const int gradId, const QPoint& globalPos) {
    if(!mScene) return;
    mContextMenuGradientId = gradId;
    const bool gradPressed = gradId < mGradients.count() && gradId >= 0;
    const auto pressedGradient = gradPressed ? mGradients.at(gradId) : nullptr;
    const auto clipboard = Document::sInstance->getPropertyClipboard();
    const bool compat = clipboard && clipboard->hasType<SceneBoundGradient>();
    QMenu menu(this);
    const auto newAct = menu.addAction("New Gradient");
    const auto pasteAct = menu.addAction("Paste Gradient");
    pasteAct->setEnabled(compat);
    QAction* pasteIntoAct = nullptr;
    QAction* copyAct = nullptr;
    QAction* duplicateAct = nullptr;
    QAction* deleteAct = nullptr;

    if(gradPressed) {
        menu.addSeparator();
        pasteIntoAct = menu.addAction("Paste Into Gradient");
        pasteIntoAct->setEnabled(compat);
        copyAct = menu.addAction("Copy Gradient");
        menu.addSeparator();
        duplicateAct = menu.addAction("Duplicate Gradient");
        menu.addSeparator();
        deleteAct = menu.addAction("Delete Gradient");
    }
    const auto selectedAction = menu.exec(globalPos);
    if(selectedAction) {
        if(selectedAction == newAct) {
            const auto newGrad = mScene->createNewGradient();
            newGrad->addColor(Qt::black);
            newGrad->addColor(Qt::white);
            setSelectedGradient(newGrad);
        } else if(selectedAction == pasteAct) {
            const auto newGrad = mScene->createNewGradient();
            clipboard->paste(newGrad);
        } else if(selectedAction == pasteIntoAct) {
            pressedGradient->clear();
            clipboard->paste(pressedGradient);
        } else if(selectedAction == copyAct) {
            const auto clipboard = enve::make_shared<PropertyClipboard>(
                        pressedGradient);
            Document::sInstance->replaceClipboard(clipboard);
        } else if(selectedAction == duplicateAct) {
            const auto newGrad = mScene->createNewGradient();
            const auto clipboard = enve::make_shared<PropertyClipboard>(pressedGradient);
            clipboard->paste(newGrad);
            setSelectedGradient(newGrad);
        } else if(selectedAction == deleteAct) {
            mScene->removeGradient(pressedGradient->ref<SceneBoundGradient>());
        }
        if(mSelectedGradient) emit triggered(mSelectedGradient);
        Document::sInstance->actionFinished();
    }
    mContextMenuGradientId = -1;
}

void DisplayedGradientsWidget::mousePressEvent(QMouseEvent *event) {
    const int gradientId = event->y()/MIN_WIDGET_DIM;
    if(event->button() == Qt::LeftButton) {
        gradientLeftPressed(gradientId);
    } else if(event->button() == Qt::RightButton) {
        gradientContextMenuReq(gradientId, event->globalPos());
        const QPoint relCursorPos = mapFromGlobal(QCursor::pos());
        if(relCursorPos.x() < 0 || relCursorPos.y() < 0 ||
           relCursorPos.x() > width() || relCursorPos.y() > height()) {
            mHoveredGradientId = -1;
        } else {
            mHoveredGradientId = relCursorPos.y()/MIN_WIDGET_DIM;
        }
    }
    update();
}

void DisplayedGradientsWidget::mouseMoveEvent(QMouseEvent *event) {
    mHoveredGradientId = event->y()/MIN_WIDGET_DIM;
    update();
}

void DisplayedGradientsWidget::leaveEvent(QEvent *) {
    mHoveredGradientId = -1;
    update();
}
