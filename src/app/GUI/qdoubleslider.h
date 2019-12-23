// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#ifndef QDOUBLESLIDER_H
#define QDOUBLESLIDER_H

#include <QWidget>
#include <QLineEdit>
#include <QDoubleValidator>

class QDoubleSlider : public QWidget {
    Q_OBJECT
public:
    QDoubleSlider(const qreal minVal,
                  const qreal maxVal,
                  const qreal prefferedStep,
                  QWidget * const parent = nullptr);
    QDoubleSlider(const QString& name,
                  const qreal minVal,
                  const qreal maxVal,
                  const qreal prefferedStep,
                  QWidget * const parent = nullptr);
    QDoubleSlider(QWidget *parent = nullptr);
    virtual ~QDoubleSlider();

    bool eventFilter(QObject *, QEvent *event);

    void setValueSliderVisibile(bool valueSliderVisible);
    void setNameVisible(bool nameVisible);
    void setName(const QString& name);
    void setNumberDecimals(int decimals);

    void setValueNoUpdate(const qreal value);
    void updateLineEditFromValue();
    void fitWidthToContent();
    QString getValueString();

    void setValueRange(qreal min, qreal max);
    virtual void paint(QPainter *p);
    void paint(QPainter *p,
               const QColor &allFill,
               const QColor &sliderFill,
               const QColor &stroke,
               const QColor &text);
    void paint(QPainter * const p, const bool enabled);

    void setPrefferedValueStep(const qreal step);

    qreal maximum();
    qreal minimum();
    qreal value() {
        return mValue;
    }

    virtual void openContextMenu(const QPoint &globalPos) {
        Q_UNUSED(globalPos)
    }

    void setNeighbouringSliderToTheLeft(const bool bT) {
        mLeftNeighbour = bT;
        update();
    }

    void setNeighbouringSliderToTheRight(const bool bT) {
        mRightNeighbour = bT;
        update();
    }
protected:
    void paintEvent(QPaintEvent *);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    virtual void emitEditingStarted(qreal value);
    virtual void emitValueChanged(qreal value);
    virtual void emitEditingFinished(qreal value);
signals:
    void editingStarted(qreal);
    void valueChanged(qreal);
    void editingFinished(qreal);
public slots:
    void setDisplayedValue(qreal value);
protected:
    bool mLeftNeighbour = false;
    bool mRightNeighbour = false;
    int mDecimals = 3;

    QString mName = "";
    bool mShowName = false;

    QLineEdit *mLineEdit = nullptr;
    qreal mValue = 0.;
    bool mTextEdit = false;
    int mMovesCount = 0;
    bool mMouseMoved = false;
    QDoubleValidator *mValidator;
    qreal mMinValue = 0.;
    qreal mMaxValue = 0.;
    qreal mPrefferedValueStep = 1.;
    void finishTextEditing();

    QPoint mGlobalPressPos;
    int mPressX;
    qreal mPressValue;
    bool mShowValueSlider = true;
private slots:
    void lineEditingFinished();
};

#endif // QDOUBLESLIDER_H
