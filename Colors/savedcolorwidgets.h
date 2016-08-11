#ifndef SAVEDCOLORWIDGETS_H
#define SAVEDCOLORWIDGETS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

class WindowVariables;

class SavedColorWidgets : public QWidget
{
    Q_OBJECT
public:
    explicit SavedColorWidgets(QWidget *parent = 0);


private:
    QPushButton *add_collection_button = NULL;
    QVBoxLayout *main_layout = NULL;
    QVBoxLayout *widgets_layout = NULL;
    WindowVariables *window_vars = NULL;
signals:

public slots:
    void addCollection();
};

#endif // SAVEDCOLORWIDGETS_H
