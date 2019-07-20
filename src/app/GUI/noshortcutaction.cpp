#include "noshortcutaction.h"

bool NoShortcutAction::event(QEvent *e) {
    return QObject::event(e);
}
