TEMPLATE = subdirs

SUBDIRS = app \
          colorwidgetshaders \
          core \
          shaders \
          ExamplePathEffect

colorwidgetshaders.subdir = app/GUI/ColorWidgets/colorwidgetshaders
shaders.subdir = core/shaders

app.depends = core

ExamplePathEffect.depends = core
