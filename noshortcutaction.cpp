#include "noshortcutaction.h"

NoShortcutAction::NoShortcutAction(const QString &text,
                                   const QObject *receiver,
                                   const char* member,
                                   const QKeySequence &shortcut,
                                   QObject *parent) :
    QAction(text, parent) {
    connect(this, SIGNAL(triggered(bool)),
            receiver, member);
    setShortcut(shortcut);
}

bool NoShortcutAction::event(QEvent *e) {
    return QObject::event(e);
}
