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
    explicit SavedColorWidgets(QWidget *parent = nullptr);


private:
    QPushButton *add_collection_button = nullptr;
    QVBoxLayout *main_layout = nullptr;
    QVBoxLayout *widgets_layout = nullptr;
    WindowVariables *window_vars = nullptr;
signals:

public slots:
    void addCollection();
};

#endif // SAVEDCOLORWIDGETS_H
