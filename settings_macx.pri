
QMAKE_CXXFLAGS = -mmacosx-version-min=10.7 -std=gnu0x -stdlib=libc+
CONFIG +=c++11

BOOST_DIR = /opt/local/include
BOOST_LIBDIR = /opt/local/lib
BOOST_LIB =

# Yes, boost sucks, they still don't have a pc like for pkg-config https://svn.boost.org/trac/boost/ticket/1094
#CONFIG += link_pkgconfig
#PKGCONFIG += boost

INCLUDEPATH += "$${BOOST_DIR}"
LIBS += -L"$${BOOST_LIBDIR}" $${BOOST_LIBS}
HEADERS += "$${BOOST_DIR}"
