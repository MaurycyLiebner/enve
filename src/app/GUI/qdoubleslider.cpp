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

#include "qdoubleslider.h"
#include <QPainter>
#include <QKeyEvent>
#include <QJSEngine>

#include "pointhelpers.h"
#include "mainwindow.h"
#include "GUI/global.h"

QDoubleSlider::QDoubleSlider(const qreal minVal, const qreal maxVal,
                             const qreal prefferedStep,
                             QWidget * const parent) : QWidget(parent) {
    mMinValue = minVal;
    mMaxValue = maxVal;
    mPrefferedValueStep = prefferedStep;
    setDisplayedValue(minVal);
    updateValueString();

    setFixedHeight(MIN_WIDGET_DIM);
    mLineEdit = new QLineEdit(mValueString, this);
    mLineEdit->setAttribute(Qt::WA_TranslucentBackground);
    mLineEdit->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                             "color: black;");
    mLineEdit->setFocusPolicy(Qt::NoFocus);
    mLineEdit->installEventFilter(this);
    installEventFilter(this);
    mLineEdit->setFixedHeight(MIN_WIDGET_DIM);
    mLineEdit->hide();

    connect(mLineEdit, &QLineEdit::editingFinished,
            this, &QDoubleSlider::lineEditingFinished);

    fitWidthToContent();
    setContentsMargins(0, 0, 0, 0);
}

QDoubleSlider::QDoubleSlider(const QString &name,
                             const qreal minVal,
                             const qreal maxVal,
                             const qreal prefferedStep,
                             QWidget * const parent) :
    QDoubleSlider(minVal, maxVal, prefferedStep, parent) {
    setName(name);
}

QDoubleSlider::QDoubleSlider(QWidget *parent) :
    QDoubleSlider(0, 100, 1, parent) {}

QDoubleSlider::~QDoubleSlider() {
    removeEventFilter(this);
    mLineEdit->removeEventFilter(this);
}

void QDoubleSlider::setValueSliderVisible(const bool valueSliderVisible) {
    mShowValueSlider = valueSliderVisible;
    update();
}

void QDoubleSlider::setNameVisible(const bool nameVisible) {
    mShowName = nameVisible;
    fitWidthToContent();
}

void QDoubleSlider::setName(const QString &name) {
    mName = name;
    update();
}

void QDoubleSlider::setNumberDecimals(const int decimals) {
    mDecimals = decimals;
    fitWidthToContent();
    updateValueString();
}

void QDoubleSlider::updateLineEditFromValue() {
    mLineEdit->setText(getEditText());
}

bool QDoubleSlider::cancelMove() {
    if(!mTextEdit && mMouseMoved && !mCanceled) {
        cancelTransform();
        return true;
    }
    return false;
}

void QDoubleSlider::setValueRange(const qreal min, const qreal max) {
    mMinValue = min;
    mMaxValue = max;
    setDisplayedValue(clamped(mValue));
    fitWidthToContent();
}

void QDoubleSlider::paint(QPainter * const p, const bool enabled) {
    paint(p, enabled ? QColor(255, 255, 255) : QColor(220, 220, 220),
          enabled ? QColor(220, 220, 220) : QColor(200, 200, 200),
          enabled ? Qt::black : Qt::darkGray,
          enabled ? Qt::black : Qt::darkGray);
}

QString QDoubleSlider::valueToText(const qreal value) const {
    return QString::number(value, 'f', mDecimals);
}

qreal QDoubleSlider::getDValueForMouseMove(const int mouseX) const {
    return (mouseX - mLastX)*0.1*mPrefferedValueStep;
}

void QDoubleSlider::paint(QPainter *p) {
    paint(p, isEnabled());
}

void QDoubleSlider::paint(QPainter *p,
                          const QColor &allFill,
                          const QColor &sliderFill,
                          const QColor &stroke,
                          const QColor &text) {
    p->save();

    p->setRenderHint(QPainter::Antialiasing);
    QRectF boundingRect = rect().adjusted(1, 1, -1, -1);
    p->setPen(Qt::NoPen);
    p->setBrush(allFill);
    if(mLeftNeighbour) {
        p->setClipRect(width()/2, 0, width()/2, height());
    } else if(mRightNeighbour) {
        p->setClipRect(0, 0, width()/2, height());
    }
    p->drawRoundedRect(boundingRect, 5, 5);
    if(mLeftNeighbour || mRightNeighbour) {
        if(mLeftNeighbour) {
            p->setClipRect(0, 0, width()/2, height());
        } else if(mRightNeighbour) {
            p->setClipRect(width()/2, 0, width()/2, height());
        }
        p->drawRect(boundingRect);
        p->setClipping(false);
    }
    if(!mTextEdit) {
        if(mShowValueSlider) {
            p->setPen(Qt::NoPen);
            const qreal valFrac = (mValue - mMinValue)/(mMaxValue - mMinValue);
            const qreal valWidth = clamp(valFrac*width(), 0, width() - 3);
            p->setBrush(sliderFill);
            const qreal heightRemoval = qMax(0., MIN_WIDGET_DIM/2 - valWidth)*0.5;
            p->drawRoundedRect(QRectF(1, 1, valWidth, height() - 2).
                               adjusted(0, heightRemoval,
                                        0, -heightRemoval), 5, 5.);
        }
        p->setPen(text);
        if(mShowName) {
            p->drawText(rect(), Qt::AlignCenter,
                        mName + ": " + mValueString);
        } else {
            p->drawText(rect(), Qt::AlignCenter, mValueString);
        }
    }
    p->setPen(QPen(stroke, 1));
    p->setBrush(Qt::NoBrush);

    if(mLeftNeighbour) {
        p->setClipRect(width()/2, 0, width()/2, height());
    } else if(mRightNeighbour) {
        p->setClipRect(0, 0, width()/2, height());
    }
    p->drawRoundedRect(boundingRect, 5, 5);
    if(mLeftNeighbour || mRightNeighbour) {
        if(mLeftNeighbour) {
            boundingRect.adjust(-1, 0, 0, 0);
            p->setClipRect(0, 0, width()/2, height());
        } else if(mRightNeighbour) {
            boundingRect.adjust(0, 0, 1, 0);
            p->setClipRect(width()/2, 0, width()/2, height());
        }
        p->drawRect(boundingRect);
        p->setClipping(false);
    }

    p->restore();
}

void QDoubleSlider::startTransform(const qreal value) {
    emit editingStarted(value);
}

void QDoubleSlider::setValue(const qreal value) {
    setDisplayedValue(value);
    emit valueEdited(mValue);
}

void QDoubleSlider::finishTransform(const qreal value) {
    if(QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
    Actions::sInstance->finishSmoothChange();
    emit editingFinished(value);
}

void QDoubleSlider::cancelTransform() {
    if(QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
    Actions::sInstance->finishSmoothChange();
    Document::sInstance->actionFinished();
    mCanceled = true;
    emit editingCanceled();
}

void QDoubleSlider::updateValueString() {
    mValueString = valueToText(mValue);
    update();
}

void QDoubleSlider::setDisplayedValue(const qreal value) {
    const qreal clampedValue = clamped(value);
    if(isZero4Dec(mValue - clampedValue)) return;
    mValue = clampedValue;
    updateValueString();
}

void QDoubleSlider::fitWidthToContent() {
    const QFontMetrics fm(QApplication::font());
    QString textMax;
    QString textMin;
    if(mShowName) {
        textMax = mName + ": " + valueToText(mMaxValue);
        textMin = mName + ": " + valueToText(mMinValue);
    } else {
        textMax = valueToText(mMaxValue);
        textMin = valueToText(mMinValue);
    }
    const int textWidth = qMax(fm.width(textMax), fm.width(textMin));
    const int maxWidth = (mShowName ? 6 : 3)*MIN_WIDGET_DIM;
    const int padding = MIN_WIDGET_DIM/2;
    const int newWidth = qMin(maxWidth, textWidth + padding);
    setFixedWidth(newWidth);
    mLineEdit->setFixedWidth(newWidth);
}

void QDoubleSlider::paintEvent(QPaintEvent *) {
    QPainter p(this);
    paint(&p);
    p.end();
}

void QDoubleSlider::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event)
}

void QDoubleSlider::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        if(cancelMove()) {}
        else openContextMenu(event->globalPos());
    } else if(event->button() == Qt::LeftButton) {
        grabMouse();
        grabKeyboard();
        mCanceled = false;
        mGlobalPressPos = event->globalPos();
        mLastX = event->globalX();
        mLastValue = mValue;
    }
}

void QDoubleSlider::setPrefferedValueStep(const qreal step) {
    mPrefferedValueStep = step;
}

void QDoubleSlider::setIsRightSlider(const bool value) {
    mLeftNeighbour = value;
    update();
}

void QDoubleSlider::setIsLeftSlider(const bool value) {
    mRightNeighbour = value;
    update();
}

void QDoubleSlider::mouseMoveEvent(QMouseEvent *event) {
    if(!mMouseMoved) startTransform(mValue);

    const qreal dValue = getDValueForMouseMove(event->globalX());
    const qreal newValue = clamped(mLastValue + dValue);
    mLastValue = newValue;
    setValue(newValue);

    if(!mMouseMoved) {
        QApplication::setOverrideCursor(Qt::BlankCursor);
        mMouseMoved = true;
    }
    cursor().setPos(mGlobalPressPos);
}

#include <QApplication>
bool QDoubleSlider::eventFilter(QObject *, QEvent *event) {
    const auto etype = event->type();
    if(etype == QEvent::Paint) {
        return false;
    } else if(etype == QEvent::KeyPress) {
        const auto keyEvent = static_cast<QKeyEvent*>(event);
        const auto ekey = keyEvent->key();
        if(mTextEdit) {
            if(ekey == Qt::Key_Escape) mCanceled = true;
            if(ekey == Qt::Key_Return ||
               ekey == Qt::Key_Enter ||
               ekey == Qt::Key_Escape) {
                finishTextEditing();
            }
        } else if(ekey == Qt::Key_Escape) {
            return !cancelMove();
        }
        return !mTextEdit;
    } else if(etype == QEvent::KeyRelease) {
        return !mTextEdit;
    } else if(etype == QEvent::MouseButtonPress) {
        const auto mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() != Qt::LeftButton) return false;
        mMouseMoved = false;
        mMovesCount = 0;
        if(mTextEdit) {
            if(!rect().contains(mouseEvent->pos()) ) {
                finishTextEditing();
            }
        } else {
            Actions::sInstance->startSmoothChange();
            mousePressEvent(mouseEvent);
        }
        return !mTextEdit;
    } else if(etype == QEvent::MouseButtonRelease) {
        const auto mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::LeftButton) {
            releaseMouse();
            releaseKeyboard();
        }
        if(mCanceled) return true;
        if(mouseEvent->button() != Qt::LeftButton) return false;
        if(!mTextEdit) {
            if(mMouseMoved) {
                mMouseMoved = false;
                finishTransform(mValue);
                Document::sInstance->actionFinished();
            } else {
                updateLineEditFromValue();
                mCanceled = false;
                mLineEdit->setCursor(Qt::IBeamCursor);
                mLineEdit->show();
                mLineEdit->setFocus();
                mLineEdit->selectAll();
                mLineEdit->grabMouse();
            }
        }
        return !mTextEdit;
    } else if(etype == QEvent::MouseMove) {
        if(mCanceled) return true;
        const auto mouseEvent = static_cast<QMouseEvent*>(event);
        if(!(mouseEvent->buttons() & Qt::LeftButton)) return false;
        if(!mTextEdit) {
            mMovesCount++;
            if(mMovesCount > 2) {
                mouseMoveEvent(static_cast<QMouseEvent*>(event));
                Document::sInstance->updateScenes();
            }
        }
        return !mTextEdit;
    } else if(etype == QEvent::FocusOut) {
        mTextEdit = false;
        mLineEdit->hide();
    } else if(etype == QEvent::FocusIn) {
        mTextEdit = true;
    }/* else if(event->type() == QEvent::Wheel) {
        if(mWheelEnabled || mTextEdit) {
            if(obj == mLineEdit) return true;
            emitEditingStarted(mValue);

            QWheelEvent *wheelEvent = (QWheelEvent*)event;
            if(wheelEvent->delta() > 0) {
                setValueNoUpdate(mValue + mPrefferedValueStep);
            } else {
                setValueNoUpdate(mValue - mPrefferedValueStep);
            }
            if(mTextEdit) {
                updateLineEditFromValue();
            }
            update();
            event->setAccepted(true);


            emitValueChanged(mValue);
            emitEditingFinished(mValue);
            MainWindow::getInstance()->callUpdateSchedulers();
            return true;
        } else {
            return false;
        }
    }*/
    return false;
}

void QDoubleSlider::finishTextEditing() {
    mLineEdit->deselect();
    mLineEdit->clearFocus();
    mLineEdit->editingFinished();
    Document::sInstance->actionFinished();
}

void QDoubleSlider::lineEditingFinished() {
    mLineEdit->releaseMouse();
    mLineEdit->setCursor(Qt::ArrowCursor);
    setCursor(Qt::ArrowCursor);
    if(mCanceled) return;
    const QString text = mLineEdit->text();
    QJSEngine engine;
    const auto result = engine.evaluate(text);
    if(!result.isNumber()) return;
    const qreal newValue = clamped(result.toNumber());
    startTransform(newValue);
    setValue(newValue);
    finishTransform(newValue);
}
