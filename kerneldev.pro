TEMPLATE = subdirs
TARGET = demo
CONFIG += ordered
INCLUDEPATH += /usr/include/boost
SUBDIRS     = \
    libraries \
    demo

exists( custom.pro ):include( custom.pro )
