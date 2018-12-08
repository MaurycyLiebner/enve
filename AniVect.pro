TEMPLATE = subdirs

SUBDIRS = app \
    core \
    shaders

app.depends = core
