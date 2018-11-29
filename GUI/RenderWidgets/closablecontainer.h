#ifndef CLOSABLECONTAINER_H
#define CLOSABLECONTAINER_H
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
class QCheckBox;

class ClosableContainer : public QWidget {
    Q_OBJECT
public:
    explicit ClosableContainer(QWidget *parent = nullptr);

    void setLabelWidget(QWidget *widget);
    void addContentWidget(QWidget *widget);
    void setCheckable(const bool &check);
    void setChecked(const bool &check);
    bool isChecked();
protected:
    QCheckBox *mCheckBox = nullptr;
    QHBoxLayout *mMainLayout = new QHBoxLayout();
    QVBoxLayout *mContLayout = new QVBoxLayout();
    QWidget *mLabelWidget = nullptr;
    QList<QWidget*> mContWidgets;
    QPushButton *mContentArrow = new QPushButton();
signals:

public slots:
    void setContentVisible(const bool &bT);
};

#endif // CLOSABLECONTAINER_H
