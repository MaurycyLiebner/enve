TEMPLATE = subdirs

SUBDIRS = app \
    core \
    libmypaint \
    shaders \
    colorwidgetshaders

colorwidgetshaders.subdir = app/GUI/ColorWidgets/colorwidgetshaders

app.depends = core
app.depends = libmypaint
