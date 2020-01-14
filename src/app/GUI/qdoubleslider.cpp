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
#include "pointhelpers.h"
#include "mainwindow.h"
#include "GUI/global.h"

QDoubleSlider::QDoubleSlider(const qreal minVal, const qreal maxVal,
                             const qreal prefferedStep,
                             QWidget * const parent) : QWidget(parent) {
    mValue = minVal;
    mMinValue = minVal;
    mMaxValue = maxVal;
    mPrefferedValueStep = prefferedStep;
    setFixedHeight(MIN_WIDGET_DIM);
    mLineEdit = new QLineEdit(QLocale().toString(mValue, 'f', mDecimals), this);
    mLineEdit->setAttribute(Qt::WA_TranslucentBackground);
    mLineEdit->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                             "color: black;");
    mLineEdit->setFocusPolicy(Qt::NoFocus);
    mLineEdit->installEventFilter(this);
    installEventFilter(this);
    mLineEdit->setFixedHeight(MIN_WIDGET_DIM);
    mValidator = new QDoubleValidator(minVal, maxVal, 3, this);
    mValidator->setNotation(QDoubleValidator::StandardNotation);
    mLineEdit->setValidator(mValidator);
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
//    delete mLineEdit;
//    mLineEdit = nullptr;
//    delete mValidator;
//    mValidator = nullptr;
}

void QDoubleSlider::setValueSliderVisibile(bool valueSliderVisible) {
    mShowValueSlider = valueSliderVisible;
    update();
}

void QDoubleSlider::setNameVisible(bool nameVisible) {
    mShowName = nameVisible;
    fitWidthToContent();
}

void QDoubleSlider::setName(const QString &name) {
    mName = name;
    setNameVisible(true);
    fitWidthToContent();
}

void QDoubleSlider::setNumberDecimals(int decimals) {
    mDecimals = decimals;
    //updateLineEditFromValue();
    fitWidthToContent();
}

void QDoubleSlider::setValueNoUpdate(const qreal value) {
    mValue = clamp(value, mMinValue, mMaxValue);
}

void QDoubleSlider::updateLineEditFromValue() {
    mLineEdit->setText(QLocale().toString(mValue, 'f', mDecimals));
}

QString QDoubleSlider::getValueString() {
    return QLocale().toString(mValue, 'f', mDecimals);
}

void QDoubleSlider::setValueRange(qreal min, qreal max) {
    mValidator->setRange(min, max, 3);
    mMinValue = min;
    mMaxValue = max;
    setValueNoUpdate(mValue);
    //updateLineEditFromValue();
    fitWidthToContent();
}

void QDoubleSlider::paint(QPainter * const p, const bool enabled) {
    paint(p, enabled ? QColor(255, 255, 255) : QColor(220, 220, 220),
          enabled ? QColor(220, 220, 220) : QColor(200, 200, 200),
          enabled ? Qt::black : Qt::darkGray,
          enabled ? Qt::black : Qt::darkGray);
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
                        mName + ": " + getValueString());
        } else {
            p->drawText(rect(), Qt::AlignCenter, getValueString());
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

void QDoubleSlider::emitEditingStarted(qreal value) {
    emit editingStarted(value);
}

void QDoubleSlider::emitValueChanged(qreal value) {
    emit valueChanged(value);
}

void QDoubleSlider::emitEditingFinished(qreal value) {
    emit editingFinished(value);
}

void QDoubleSlider::emitEditingCanceled() {
    emit editingCanceled();
}

void QDoubleSlider::setDisplayedValue(qreal value) {
    setValueNoUpdate(value);
    update();
}

void QDoubleSlider::fitWidthToContent() {
    QFontMetrics fm = QFontMetrics(QFont());
    QString textMax;
    QString textMin;
    if(mShowName) {
        textMax = mName + ": " + QString::number(mMaxValue, 'f', mDecimals);
        textMin = mName + ": " + QString::number(mMinValue, 'f', mDecimals);
    } else {
        textMax = QString::number(mMaxValue, 'f', mDecimals);
        textMin = QString::number(mMinValue, 'f', mDecimals);
    }
    int textWidth = qMax(fm.width(textMax), fm.width(textMin));
    int newWidth = qMin(3*MIN_WIDGET_DIM,
                        textWidth + textWidth%2 + MIN_WIDGET_DIM/2);
    int minWidth = qMax(0, newWidth - MIN_WIDGET_DIM);
    setMinimumWidth(minWidth);
    mLineEdit->setMinimumWidth(minWidth);
    setMaximumWidth(newWidth);
    mLineEdit->setMaximumWidth(newWidth);
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
        if(mMouseMoved) {
            emitEditingCanceled();
            mCanceled = true;
            setCursor(Qt::ArrowCursor);
        } else openContextMenu(event->globalPos());
    } else if(event->button() == Qt::LeftButton) {
        mCanceled = false;
        setCursor(Qt::BlankCursor);
        mPressX = event->x();
        mGlobalPressPos = event->globalPos();
        mPressValue = mValue;
    }
}

void QDoubleSlider::setPrefferedValueStep(const qreal step) {
    mPrefferedValueStep = step;
}

qreal QDoubleSlider::maximum() {
    return mMaxValue;
}

qreal QDoubleSlider::minimum() {
    return mMinValue;
}

void QDoubleSlider::mouseMoveEvent(QMouseEvent *event) {
    if(!mMouseMoved) emitEditingStarted(mValue);
    const qreal dValue = (event->x() - mPressX)*0.1*mPrefferedValueStep;
    setValueNoUpdate(mPressValue + dValue);
    update();

    mPressValue = mValue;
    cursor().setPos(mGlobalPressPos);

    emitValueChanged(mValue);
    mMouseMoved = true;
}

#include <QApplication>
bool QDoubleSlider::eventFilter(QObject *, QEvent *event) {
    if(event->type() == QEvent::Paint) {
        return false;
    } else if(event->type() == QEvent::KeyPress) {
        const auto keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Return ||
           keyEvent->key() == Qt::Key_Enter) {
            finishTextEditing();
        } else if((keyEvent->key() == Qt::Key_Period ||
                  keyEvent->key() == Qt::Key_Comma) && mTextEdit) {
            QString currentText = mLineEdit->text();
            int int2;
            QString currentTextPeriod = currentText + ".";
            if(mValidator->validate(currentTextPeriod, int2) !=
                    QValidator::Invalid) {
                mLineEdit->setText(currentTextPeriod);
                return true;
            }
            QString currentTextComa = currentText + ",";
            if(mValidator->validate(currentTextComa, int2) !=
                    QValidator::Invalid) {
                mLineEdit->setText(currentTextComa);
                return true;
            }
        }
        return !mTextEdit;
    } else if(event->type() == QEvent::KeyRelease) {
        return !mTextEdit;
    } else if(event->type() == QEvent::MouseButtonPress) {
        const auto mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton) return false;
        mMouseMoved = false;
        mMovesCount = 0;
        if(mTextEdit) {
            if(!rect().contains(mouseEvent->pos()) ) {
                finishTextEditing();
//                QApplication::setOverrideCursor(QApplication::widgetAt(mouseEvent->globalPos())->cursor());
//                QApplication::restoreOverrideCursor();
            }
        } else {
            Actions::sInstance->startSmoothChange();
            mousePressEvent(mouseEvent);
        }
        return !mTextEdit;
    } else if(event->type() == QEvent::MouseButtonRelease) {
        if(mCanceled) return true;
        const auto mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton) return false;
        if(!mTextEdit) {
            Actions::sInstance->finishSmoothChange();
            if(mMouseMoved) {
                mMouseMoved = false;
                setCursor(Qt::ArrowCursor);
                emitEditingFinished(mValue);
                Document::sInstance->actionFinished();
            } else {
                updateLineEditFromValue();
                mLineEdit->setCursor(Qt::IBeamCursor);
                mLineEdit->show();
                mLineEdit->setFocus();
                mLineEdit->selectAll();
                mLineEdit->grabMouse();
            }
        }
        return !mTextEdit;
    } else if(event->type() == QEvent::MouseMove) {
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
    } else if(event->type() == QEvent::FocusOut) {
        mTextEdit = false;
        mLineEdit->hide();
    } else if(event->type() == QEvent::FocusIn) {
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
    mLineEdit->setCursor(Qt::ArrowCursor);
    setCursor(Qt::ArrowCursor);
    const QString text = mLineEdit->text();
    setValueNoUpdate(QLocale().toDouble(text));
    mLineEdit->releaseMouse();

    emitEditingStarted(mValue);
    emitValueChanged(mValue);
    emitEditingFinished(mValue);
}
