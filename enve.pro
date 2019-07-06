TEMPLATE = subdirs

SUBDIRS = app \
          colorwidgetshaders \
          core \
          shaders \
          examplePathEffect \
          exampleGpuEffect

colorwidgetshaders.subdir = app/GUI/ColorWidgets/colorwidgetshaders
shaders.subdir = core/shaders

app.depends = core

examplePathEffect.depends = core
exampleGpuEffect.depends = core
