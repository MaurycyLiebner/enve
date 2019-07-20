TEMPLATE = subdirs

SUBDIRS = app \
          colorwidgetshaders \
          core \
          shaders

colorwidgetshaders.subdir = app/GUI/ColorWidgets/colorwidgetshaders
shaders.subdir = core/shaders

app.depends = core
