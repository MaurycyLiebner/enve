#ifndef VERTICALWIDGETSSTACK_H
#define VERTICALWIDGETSSTACK_H

#include <QWidget>

class BoxesListKeysViewWidgetResizer :
        public QWidget {
    Q_OBJECT
public:
    BoxesListKeysViewWidgetResizer(QWidget *parent);

    void paintEvent(QPaintEvent *);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

    void enterEvent(QEvent *);

    void leaveEvent(QEvent *);

    void setAboveWidget(QWidget *aboveWidget);
    void setBelowWidget(QWidget *belowWidget);

    void setAboveResizer(BoxesListKeysViewWidgetResizer *aboveResizer);
    void setBelowResizer(BoxesListKeysViewWidgetResizer *belowResizer);
    void displace(int totDy);
signals:
    void finishedChanging();
private:
    BoxesListKeysViewWidgetResizer *mAboveResizer = NULL;
    BoxesListKeysViewWidgetResizer *mBelowResizer = NULL;

    QWidget *mAboveWidget;
    QWidget *mBelowWidget;
    bool mHover = false;
    bool mPressed = false;
    int mPressY;
};

class VerticalWidgetsStack :
        public QWidget {
    Q_OBJECT
public:
    VerticalWidgetsStack(QWidget *parent = 0);

    void updateSizesAndPositions();
    void updateResizers();
    void appendWidget(QWidget *widget);
    void insertWidget(const int &id, QWidget *widget);
    void removeWidget(QWidget *widget);

    void resizeEvent(QResizeEvent *);
    int getIdOf(QWidget *idOf);
signals:

public slots:
    void updatePercent();
private:
    QList<QWidget*> mWidgets;
    QList<qreal> mHeightPercent;
    QList<BoxesListKeysViewWidgetResizer*> mResizers;
};

#endif // VERTICALWIDGETSSTACK_H
