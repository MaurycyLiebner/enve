#ifndef QDOUBLESLIDER_H
#define QDOUBLESLIDER_H

#include <QWidget>
#include <QLineEdit>
#include <QDoubleValidator>

class QDoubleSlider : public QWidget
{
    Q_OBJECT
public:
    QDoubleSlider(qreal minVal, qreal maxVal, QWidget *parent = 0);
    QDoubleSlider(QString name, qreal minVal, qreal maxVal, QWidget *parent = 0);
    QDoubleSlider(QWidget *parent = 0);
    ~QDoubleSlider();

    bool eventFilter(QObject *obj, QEvent *event);

    void setValueSliderVisibile(bool valueSliderVisible);
    void setNameVisible(bool nameVisible);
    void setName(QString name);
    void setNumberDecimals(int decimals);

    void setValue(qreal value);
    void updateLineEditFromValue();
    void fitWidthToContent();
    QString getValueString();

    void setValueRange(qreal min, qreal max);

    virtual void paint(QPainter *p);
protected:
    void paintEvent(QPaintEvent *);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    virtual void emitValueChanged(qreal value);
    virtual void emitEditingFinished(qreal value);
signals:
    void valueChanged(qreal);
    void editingFinished(qreal);
public slots:
protected:
    int mDecimals = 3;

    QString mName;
    bool mShowName = false;

    QLineEdit *mLineEdit;
    qreal mValue;
    bool mTextEdit = false;
    int mMovesCount = 0;
    bool mMouseMoved = false;
    QDoubleValidator *mValidator;
    qreal mMinValue;
    qreal mMaxValue;
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
