#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QWidget>

class ActionButton : public QWidget
{
    Q_OBJECT
public:
    explicit ActionButton(const QString &notCheckedPix,
                          const QString &toolTip,
                          QWidget *parent = 0);
    void setCheckable(const QString &checkedPix);
    void setChecked(bool checked);
protected:
    void mousePressEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void paintEvent(QPaintEvent *);

    bool mHover = false;
    bool mChecked = false;
    bool mCheckable = false;

    QImage mCheckedPixmap;
    QImage mNotCheckedPixmap;
signals:
    void pressed();
    void toggled(bool);
public slots:
};

#endif // ACTIONBUTTON_H
