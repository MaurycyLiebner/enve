TEMPLATE = subdirs

SUBDIRS = src

build_examples {
    SUBDIRS += examples
    examples.depends = src
}
