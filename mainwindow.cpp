#include "mainwindow.h"
#include <QtSvg/QGraphicsSvgItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    graphicsScene = new QGraphicsScene(this);
    graphicsView = new QGraphicsView(graphicsScene, this);
    setCentralWidget(graphicsView);
}

MainWindow::~MainWindow()
{

}
