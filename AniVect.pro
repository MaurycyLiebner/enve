TEMPLATE = subdirs

SUBDIRS = app \
    core

app.depends = core
