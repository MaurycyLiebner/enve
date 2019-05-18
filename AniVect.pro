TEMPLATE = subdirs

SUBDIRS = app \
    core \
    shaders \
    colorwidgetshaders

colorwidgetshaders.subdir = app/GUI/ColorWidgets/colorwidgetshaders

app.depends = core
