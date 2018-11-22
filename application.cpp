#include "application.h"
#include <QMainWindow>
#include <QDockWidget>

Application::Application(int &argc, char **argv) : QApplication(argc, argv) {

}

bool Application::notify(QObject *o, QEvent *e) {
    QMainWindow* mw;
    if(o->isWidgetType() && e->type() == QEvent::MouseButtonPress && (mw = qobject_cast<QMainWindow*>(o))) {
        for(auto& ch : mw->children()) {
            if(auto dw = qobject_cast<QDockWidget*>(ch)) {
                auto pname = "_wa-QTBUG-65592";
                auto v = dw->property(pname);
                if(v.isNull()) {
                    dw->setProperty(pname, true);
                    mw->restoreDockWidget(dw);
                    auto area = mw->dockWidgetArea(dw);
                    auto orient = area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea ? Qt::Horizontal : Qt::Vertical;
                    mw->resizeDocks({dw}, { orient == Qt::Horizontal ? dw->width() : dw->height() }, orient);
                }
            }
        }
    }
    return QApplication::notify(o, e);
}
