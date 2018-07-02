#include "qdoubleslider.h"
#include <QPainter>
#include <QKeyEvent>
#include "pointhelpers.h"
#include "mainwindow.h"
#include "global.h"

QDoubleSlider::QDoubleSlider(qreal minVal, qreal maxVal,
                             qreal prefferedStep,
                             QWidget *parent) : QWidget(parent) {
    mValue = minVal;
    mMinValue = minVal;
    mMaxValue = maxVal;
    mPrefferedValueStep = prefferedStep;
    setFixedHeight(MIN_WIDGET_HEIGHT);
    mLineEdit = new QLineEdit(QString::number(mValue, 'f', mDecimals), this);
    mLineEdit->setAttribute(Qt::WA_TranslucentBackground);
    mLineEdit->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                             "color: black;");
    mLineEdit->setFocusPolicy(Qt::NoFocus);
    mLineEdit->installEventFilter(this);
    installEventFilter(this);
    mLineEdit->setFixedHeight(MIN_WIDGET_HEIGHT);
    mValidator = new QDoubleValidator(minVal, maxVal, 3, this);
    mValidator->setNotation(QDoubleValidator::StandardNotation);
    mLineEdit->setValidator(mValidator);
    mLineEdit->hide();

    connect(mLineEdit, SIGNAL(editingFinished()),
            this, SLOT(lineEditingFinished()));

    fitWidthToContent();
    setContentsMargins(0, 0, 0, 0);
}

QDoubleSlider::QDoubleSlider(QString name,
                             qreal minVal, qreal maxVal,
                             qreal prefferedStep,
                             QWidget *parent) :
    QDoubleSlider(minVal, maxVal, prefferedStep, parent) {
    setName(name);
}

QDoubleSlider::QDoubleSlider(QWidget *parent) :
    QDoubleSlider(0., 100., 1., parent)
{

}

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

void QDoubleSlider::setName(QString name) {
    mName = name;
    setNameVisible(true);
    fitWidthToContent();
}

void QDoubleSlider::setNumberDecimals(int decimals) {
    mDecimals = decimals;
    //updateLineEditFromValue();
    fitWidthToContent();
}

void QDoubleSlider::setValueNoUpdate(qreal value) {
    mValue = qclamp(value, mMinValue, mMaxValue);
}

void QDoubleSlider::updateLineEditFromValue() {
    mLineEdit->setText(QString::number(mValue, 'f', mDecimals));
}

QString QDoubleSlider::getValueString() {
    return QString::number(mValue, 'f', mDecimals);
}

void QDoubleSlider::setValueRange(qreal min, qreal max) {
    mValidator->setRange(min, max, 3);
    mMinValue = min;
    mMaxValue = max;
    setValueNoUpdate(mValue);
    //updateLineEditFromValue();
    fitWidthToContent();
}

void QDoubleSlider::paint(QPainter *p) {
    paint(p,
          QColor(255, 255, 255),
          QColor(220, 220, 220),
          Qt::black);
}

void QDoubleSlider::paint(QPainter *p,
                          const QColor &allFill,
                          const QColor &sliderFill,
                          const QColor &stroke) {
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
    p->drawRoundedRect(boundingRect, 5., 5.);
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
            qreal valWidth = qclamp((mValue - mMinValue)*width()/(mMaxValue - mMinValue),
                                    0., width() - 3.);
            p->setBrush(sliderFill);
            qreal heightRemoval = qMax(0., MIN_WIDGET_HEIGHT/2 - valWidth)*0.5;
            p->drawRoundedRect(QRectF(1., 1.,
                                      valWidth,
                                      height() - 2.).
                               adjusted(0., heightRemoval,
                                        0., -heightRemoval), 5., 5.);
        }
        p->setPen(Qt::black);
        if(mShowName) {
            p->drawText(rect(), Qt::AlignCenter,
                        mName + ": " + getValueString());
        } else {
            p->drawText(rect(), Qt::AlignCenter, getValueString());
        }
    }
    p->setPen(QPen(stroke, 1.));
    p->setBrush(Qt::NoBrush);

    if(mLeftNeighbour) {
        p->setClipRect(width()/2, 0, width()/2, height());
    } else if(mRightNeighbour) {
        p->setClipRect(0, 0, width()/2, height());
    }
    p->drawRoundedRect(boundingRect, 5., 5.);
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
    int newWidth = qMin(3*MIN_WIDGET_HEIGHT,
                        textWidth + textWidth%2 + MIN_WIDGET_HEIGHT/2);
    int minWidth = qMax(0, newWidth - MIN_WIDGET_HEIGHT);
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
    Q_UNUSED(event);
}

void QDoubleSlider::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        openContextMenu(event->globalPos());
    } else if(event->button() == Qt::LeftButton) {
        setCursor(Qt::BlankCursor);
        mPressX = event->x();
        mGlobalPressPos = event->globalPos();
        mPressValue = mValue;
    }
}

void QDoubleSlider::setPrefferedValueStep(qreal step) {
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
    qreal dValue = (event->x() - mPressX)*0.1*mPrefferedValueStep;
    setValueNoUpdate(mPressValue + dValue);
    update();

    mPressValue = mValue;
    cursor().setPos(mGlobalPressPos);

    emitValueChanged(mValue);
}

#include <QApplication>
bool QDoubleSlider::eventFilter(QObject *, QEvent *event) {
    if(event->type() == QEvent::Paint) {
        return false;
    } else if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        if(keyEvent->key() == Qt::Key_Return ||
           keyEvent->key() == Qt::Key_Enter) {
            finishTextEditing();
            MainWindow::getInstance()->callUpdateSchedulers();
        }
        return !mTextEdit;
    } else if(event->type() == QEvent::KeyRelease) {
        return !mTextEdit;
    } else if(event->type() == QEvent::MouseButtonPress) {
        mMouseMoved = false;
        mMovesCount = 0;
        QMouseEvent *mouseEvent = (QMouseEvent*) event;
        if(mTextEdit) {
            if(!rect().contains(mouseEvent->pos()) ) {
                finishTextEditing();
                MainWindow::getInstance()->callUpdateSchedulers();
//                QApplication::setOverrideCursor(QApplication::widgetAt(mouseEvent->globalPos())->cursor());
//                QApplication::restoreOverrideCursor();
            }
        } else {
            mousePressEvent(mouseEvent);
        }
        return !mTextEdit;
    } else if(event->type() == QEvent::MouseButtonRelease) {
        if(!mTextEdit) {
            if(mMouseMoved) {
                setCursor(Qt::ArrowCursor);
                emitEditingFinished(mValue);
                MainWindow::getInstance()->callUpdateSchedulers();
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
        if(!mTextEdit) {
            mMovesCount++;
            if(mMovesCount > 2) {
                mouseMoveEvent((QMouseEvent*) event);
                mMouseMoved = true;
                MainWindow::getInstance()->callUpdateSchedulers();
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
}

void QDoubleSlider::lineEditingFinished() {
    mLineEdit->setCursor(Qt::ArrowCursor);
    setCursor(Qt::ArrowCursor);
    QString text = mLineEdit->text();
    setValueNoUpdate(text.toDouble());
    mLineEdit->releaseMouse();

    emitEditingStarted(mValue);
    emitValueChanged(mValue);
    emitEditingFinished(mValue);
}
