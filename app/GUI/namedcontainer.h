#ifndef NAMEDCONTAINER_H
#define NAMEDCONTAINER_H

#include <QWidget>
#include <QLabel>
#include <QBoxLayout>

class NamedContainer : public QWidget {
public:
    explicit NamedContainer(const QString &name,
                            QWidget* widget, const bool vertical,
                            QWidget *parent = nullptr);
private:
    QLabel* mNameLabel;
    QBoxLayout* mLayout;
};

#endif // NAMEDCONTAINER_H
