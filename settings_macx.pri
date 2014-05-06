#include(boost.pri)

QT_CONFIG -= no-pkg-config
QMAKE_CXXFLAGS = -mmacosx-version-min=10.7 -std=gnu0x -stdlib=libc++ -std=c++11
CONFIG +=c++11
