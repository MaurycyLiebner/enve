#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QGraphicsView *graphicsView;
    QGraphicsScene *graphicsScene;
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
