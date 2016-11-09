#include "qdoubleslider.h"
#include <QPainter>
#include <QKeyEvent>
#include "pointhelpers.h"

QDoubleSlider::QDoubleSlider(qreal minVal, qreal maxVal, QWidget *parent) :
    QWidget(parent)
{
    mValue = minVal;
    mMinValue = minVal;
    mMaxValue = maxVal;
    setFixedHeight(20);
    mLineEdit = new QLineEdit(QString::number(mValue, 'f', mDecimals), this);
    mLineEdit->setAttribute(Qt::WA_TranslucentBackground);
    mLineEdit->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
    mLineEdit->setFocusPolicy(Qt::NoFocus);
    mLineEdit->installEventFilter(this);
    installEventFilter(this);
    mLineEdit->setFixedHeight(20);
    mValidator = new QDoubleValidator(minVal, maxVal, 3, this);
    mValidator->setNotation(QDoubleValidator::StandardNotation);
    mLineEdit->setValidator(mValidator);
    mLineEdit->hide();

    connect(mLineEdit, SIGNAL(editingFinished()),
            this, SLOT(lineEditingFinished()));

    fitWidthToContent();
}

QDoubleSlider::QDoubleSlider(QString name,
                             qreal minVal, qreal maxVal,
                             QWidget *parent) :
    QDoubleSlider(minVal, maxVal,parent)
{
    setName(name);
}

QDoubleSlider::QDoubleSlider(QWidget *parent) : QDoubleSlider(0., 100., parent)
{

}

QDoubleSlider::~QDoubleSlider()
{
    removeEventFilter(this);
    delete mLineEdit;
    mLineEdit = NULL;
    delete mValidator;
    mValidator = NULL;
}

void QDoubleSlider::setValueSliderVisibile(bool valueSliderVisible)
{
    mShowValueSlider = valueSliderVisible;
    update();
}

void QDoubleSlider::setNameVisible(bool nameVisible)
{
    mShowName = nameVisible;
    fitWidthToContent();
}

void QDoubleSlider::setName(QString name)
{
    mName = name;
    setNameVisible(true);
    fitWidthToContent();
}

void QDoubleSlider::setNumberDecimals(int decimals)
{
    mDecimals = decimals;
    updateLineEditFromValue();
    fitWidthToContent();
}

void QDoubleSlider::setValue(qreal value)
{
    mValue = qclamp(value, mMinValue, mMaxValue);
}

void QDoubleSlider::updateLineEditFromValue()
{
    mLineEdit->setText(QString::number(mValue, 'f', mDecimals));
}

QString QDoubleSlider::getValueString() {
    return QString::number(mValue, 'f', mDecimals);
}

void QDoubleSlider::setValueRange(qreal min, qreal max)
{
    mMinValue = min;
    mMaxValue = max;
    setValue(mValue);
    updateLineEditFromValue();
    fitWidthToContent();
}

void QDoubleSlider::fitWidthToContent()
{
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
    int newWidth = textWidth + textWidth%2 + 10;
    setFixedWidth(newWidth);
    mLineEdit->setFixedWidth(newWidth);
}

void QDoubleSlider::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.fillRect(rect(), QColor(245, 245, 245));
    if(!mTextEdit) {
        if(mShowValueSlider) {
            qreal valWidth = mValue*width()/(mMaxValue - mMinValue);
            p.fillRect(QRectF(0., 0., valWidth, height()), QColor(255, 200, 200));
        }
        if(mShowName) {
            p.drawText(rect(), Qt::AlignCenter, mName + ": " + getValueString());
        } else {
            p.drawText(rect(), Qt::AlignCenter, getValueString());
        }
    }
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    p.end();
}

void QDoubleSlider::mouseDoubleClickEvent(QMouseEvent *event)
{
}

void QDoubleSlider::mousePressEvent(QMouseEvent *event)
{
    mPressX = event->x();
    mPressValue = mValue;
}

void QDoubleSlider::mouseMoveEvent(QMouseEvent *event)
{
    qreal dValue = (event->x() - mPressX)*0.008*(mMaxValue - mMinValue);
    setValue(mPressValue + dValue);
    update();

    emit valueChanged(mValue);
}

#include <QApplication>
bool QDoubleSlider::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::Paint) {
        return false;
    } else if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        if(keyEvent->key() == Qt::Key_Return ||
           keyEvent->key() == Qt::Key_Enter) {
            finishTextEditing();
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
                emit editingFinished(mValue);
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
                mMouseMoved = true;
                mouseMoveEvent((QMouseEvent*) event);
            }
        }
        return !mTextEdit;
    } else if(event->type() == QEvent::FocusOut) {
        mTextEdit = false;
        mLineEdit->hide();
    } else if(event->type() == QEvent::FocusIn) {
        mTextEdit = true;
    } else if(event->type() == QEvent::Wheel) {
        if(obj == mLineEdit) return true;
        QWheelEvent *wheelEvent = (QWheelEvent*)event;
        if(wheelEvent->delta() > 0) {
            setValue(mValue + mPrefferedValueStep);
        } else {
            setValue(mValue - mPrefferedValueStep);
        }
        if(mTextEdit) {
            updateLineEditFromValue();
        }
        update();
        event->setAccepted(true);
        return true;
    }
    return false;
}

void QDoubleSlider::finishTextEditing()
{
    mLineEdit->deselect();
    mLineEdit->clearFocus();
    mLineEdit->editingFinished();
}

void QDoubleSlider::lineEditingFinished()
{
    mLineEdit->setCursor(Qt::ArrowCursor);
    QString text = mLineEdit->text();
    setValue(text.toDouble());
    mLineEdit->releaseMouse();

    emit valueChanged(mValue);
    emit editingFinished(mValue);
}
