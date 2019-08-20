#ifndef CLOSABLECONTAINER_H
#define CLOSABLECONTAINER_H
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
class QCheckBox;

class ClosableContainer : public QWidget {
public:
    explicit ClosableContainer(QWidget *parent = nullptr);

    void setLabelWidget(QWidget *widget);
    void addContentWidget(QWidget *widget);
    void setCheckable(const bool check);
    void setChecked(const bool check);
    bool isChecked();

    void setContentVisible(const bool visible);
protected:
    QCheckBox *mCheckBox = nullptr;
    QHBoxLayout *mMainLayout = new QHBoxLayout();
    QVBoxLayout *mVLayout = new QVBoxLayout();
    QWidget *mLabelWidget = nullptr;
    QWidget* mContWidget = new QWidget(this);
    QVBoxLayout *mContLayout = new QVBoxLayout();
    QList<QWidget*> mContWidgets;
    QPushButton *mContentArrow = new QPushButton();
};

#endif // CLOSABLECONTAINER_H
