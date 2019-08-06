#include "noshortcutaction.h"
#include <QEvent>

bool NoShortcutAction::event(QEvent *e) {
    if(e->type() == QEvent::Shortcut) return true;
    else return QAction::event(e);
}
