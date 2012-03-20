TEMPLATE = subdirs
TARGET = demo
CONFIG += ordered

SUBDIRS     = \
    libraries \
    demo

exists( custom.pro ):include( custom.pro )
