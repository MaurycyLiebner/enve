#ifndef BOOKMARKEDWIDGET_H
#define BOOKMARKEDWIDGET_H

#include <QWidget>
#include <QPushButton>

class BookmarkedWidget : public QWidget {
public:
    explicit BookmarkedWidget(const bool vertical,
                              const int dimension,
                              QWidget *parent = nullptr);

    void addWidget(QWidget* const wid);
    void removeWidget(QWidget* const wid);
    QWidget* getWidget(const int id) const;
    int count() const
    { return mWidgets.count(); }
    void updateSize();
protected:
    void resizeEvent(QResizeEvent *event);
private:
    void updateLayout();

    const bool mVertical;
    const int mDimension;
    QPushButton* mUpArrow;
    QPushButton* mDownArrow;
    QList<QWidget*> mWidgets;
    int mFirstViewed = 0;
    int mLastViewed = 0;
};

#endif // BOOKMARKEDWIDGET_H
