#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H
#include <QWidget>

#include "canvasbase.h"

class BookmarkedWidget;

class CentralWidget : public QWidget {
public:
    CentralWidget(BookmarkedWidget* left,
                  QWidget* central,
                  QWidget* parent = nullptr);

    void setSidesVisibilitySetting(const bool vis);
    void setCanvasMode(const CanvasMode mode);
protected:
    void resizeEvent(QResizeEvent *event);
private:
    void updateSideWidgetsVisibility();
    void updateSizeWidgetsSize();

    bool mVisibilitySetting = true;
    CanvasMode mMode = CanvasMode::boxTransform;
    BookmarkedWidget* const mLeft;
    QWidget* const mCentral;
    //QWidget* mRight;
};

#endif // CENTRALWIDGET_H
