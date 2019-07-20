#ifndef ARRAYWIDGET_H
#define ARRAYWIDGET_H

#include <QWidget>

class QBoxLayout;
class QPushButton;

class ArrayWidget : public QWidget {
    Q_OBJECT
public:
    explicit ArrayWidget(const Qt::Orientation& orientation,
                         QWidget *parent = nullptr);

    QWidget* getWidgetAt(const int i) const {
        return mArrayWidgets.at(i);
    }

    int widgetsCount() const {
        return mArrayWidgets.count();
    }

    void setOrientation(const Qt::Orientation& orientation);

    void appendWidget(QWidget* wid);

    void prependWidget(QWidget* wid);

    void insertWidget(const int i, QWidget* wid);

    void removeRedundantWidgets();

    template <typename T>
    bool removeWidget(T tester) {
        for(QWidget* wid : mArrayWidgets) {
            if(tester(wid)) {
                mArrayWidgets.removeOne(wid);
                delete wid;
                return true;
            }
        }
        return false;
    }

    bool removeWidget(QWidget* widget);

    template <typename T>
    bool removeWidgets(T tester) {
        for(int i = 0; i < mArrayWidgets.count(); i++) {
            QWidget* wid = mArrayWidgets.at(i);
            if(tester(wid)) {
                mArrayWidgets.removeOne(wid);
                delete wid;
                i--;
            }
        }
        return false;
    }

    template <typename T>
    void sortWidgets(T tester) {
        std::sort(mArrayWidgets.begin(), mArrayWidgets.end(), tester);
        updateWidgetsVisibility();
    }

    void incFirstWidgetId(const int inc = 1);

    void decFirstWidgetId(const int dec = 1);

    void setFirstWidgetId(const int id);

    template <typename T>
    bool hasWidget(T tester) {
        for(QWidget* wid : mArrayWidgets) {
            if(tester(wid)) return true;
        }
        return false;
    }

    void setMaxCount(const int maxCount);
protected:
    void resizeEvent(QResizeEvent* event);

    virtual void connectChildWidget(QWidget* wid);
signals:

public slots:

private slots:
    void nextButtonPressed();

    void prevButtonPressed();
private:
    void updateButtonIcons();
    void updateWidgetsVisibility();
    void updateWidgetsHLayout();
    void updateWidgetsVLayout();

    int mMaxShownCount = 5;
    int mMaxCount = 10;
    int mFirstId = 0;
    Qt::Orientation mOrientation;
    QList<QWidget*> mArrayWidgets;

    QBoxLayout* mMainLayout = nullptr;

    QPushButton* mNextButton;
    QPushButton* mPrevButton;
};

#endif // ARRAYWIDGET_H
