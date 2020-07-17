#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>

#include "aboutdialog.h"

#include <boost/version.hpp>
#include "version.h"
#include <tinysplinecxx.cxx>
#include <lua.h>
#include <drw_base.h>
extern "C"
{
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}
#include <GL/glew.h>

#if __linux__
    #include <sys/utsname.h>
    #include <stdlib.h>

QString getOsName()
{
    QString out;
    struct utsname buffer;
    if (uname(&buffer) != 0) {
       return "Linux";
    }
    out += "system name: " + QString(buffer.sysname) + "<br/>";
    out += "release: " + QString(buffer.release) + "<br/>";
    out += "version: " + QString(buffer.version) + "<br/>";
    out += "machine: " + QString(buffer.machine);
    return out;
}
#elif _WIN32 || _WIN64
    // Not tested
    #include <windows.h>

QString getOsName()
{
    Qstring out;

    #ifdef _WIN32
    out += "32-bit";
    #elif _WIN64
    out += "64-bit";
    #endif

    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (GetVersionEx(&vi) == 0) return out;
    switch (vi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
        out += " Windows 3.x";
    case VER_PLATFORM_WIN32_WINDOWS:
        out += vi.dwMinorVersion == 0 ? " Windows 95" : " Windows 98";
    case VER_PLATFORM_WIN32_NT:
        out += " Windows NT";
    default:
        out += " Unknown";
    }
    return out;
}
#else
//From https://stackoverflow.com/questions/15580179/how-do-i-find-the-name-of-an-operating-system
QString getOsName()
{
    #if __APPLE__ || __MACH__
    return "Mac OSX";
    #elif __FreeBSD__
    return "FreeBSD";
    #elif __unix || __unix__
    return "Unix";
    #else
    return "Other";
    #endif
}
#endif

QString getCompiler(){
   return 
        #if defined(Q_CC_CLANG)
            QString("Clang %1.%2.%3").arg(__clang_major__).arg(__clang_minor__).arg(__clang_patchlevel__)
        #elif defined(Q_CC_GNU)
            QString("GNU GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__)
        #elif defined(Q_CC_MSVC)
            QString("Microsoft Visual C++")
        #else
            QString("other")
        #endif
    ;
}

QString getLuaVersion(){
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);//add function to return version, can this be done directly?
    luaL_dostring(L,"function version() return _VERSION end");
    lua_getglobal(L, "version");
    lua_call(L, 0, 1);
    QString lua_version = lua_tostring(L, -1);;
    lua_pop(L, 1);
    lua_close(L);
    return lua_version;
}

QString getGLVersion(){
    //Assume init is already called
    int gl_major = 0;
    int gl_minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    return QString("%1.%2").arg(gl_major).arg(gl_minor);
}

using namespace lc::ui::dialog;
//Create string of information
QString getLCADInfo(){
    return 
        QString("Version: %1.%2").arg(VERSION_MAJOR).arg(VERSION_MINOR) + "<br/>" +
        QString("Compiler: %1").arg(getCompiler()) + "<br/>" +
        QString("Compiled on: %1").arg(BUILD_DATE) + "<br/>" +
        QString("Build info: %1").arg(BUILD_INFO)
    ;
}

QString getExtInfo(){
    return
        QString("Qt Version: %1").arg(qVersion()) + "<br/>" +
        QString("Boost Version: %1.%2.%3").arg(BOOST_VERSION / 100000).arg(BOOST_VERSION / 100 % 1000).arg(BOOST_VERSION % 100) + "\n" +
        QString("Lua Version: %1").arg(getLuaVersion()) + "<br/>" + 
	QString("libdxfrw Version: %1").arg(DRW_VERSION) + "<br/>" + 
	QString("OpenGL Version: %1").arg(getGLVersion()) + "<br/>" +
	QString("CMake Version: %1").arg(CMAKE_VERSION)
    ;
}


AboutDialog::AboutDialog(QWidget* parent) :
        QDialog(parent){
    //Copied from LibreCAD_2/src/main/qc_application window,
    // and modified
    setWindowTitle(tr("About"));

    auto layout = new QVBoxLayout;
    setLayout(layout);
    _info = QString("\
		<b>LibreCAD 3</b><br/>%1<br/><br/>\
		<b>External Dependencies</b><br/>%2<br/><br/>\
		<b>OS</b><br/>%3<br/>%4"
	)
	.arg(getLCADInfo())
	.arg(getExtInfo())
	.arg(getOsName()).arg(
		#if QT_VERSION >= 0x050400
			QString("System") + ": " + QSysInfo::prettyProductName() + "<br/>" + 
			QString("Build Architecture") + ": " + QSysInfo::buildCpuArchitecture() + "<br/>" + 
			QString("Current Architecture") + ": " + QSysInfo::currentCpuArchitecture() + "<br/>" + 
			QString("Kernel Type") + ": " + QSysInfo::kernelType() + "<br/>" + 
			QString("Kernel Version") + ": " + QSysInfo::kernelVersion() + "<br/>" + 
			QString("Product Type") + ": " + QSysInfo::productType() + "<br/>" + 
			QString("Product Version") + ": " + QSysInfo::productVersion()
		#endif		
	);

    auto app_info = new QLabel(_info);
    app_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(app_info);

    QString links
    (
        QString("<a href=\"https://github.com/LibreCAD/LibreCAD_3/graphs/contributors\">%1</a>").arg(tr("Contributors"))
        + "<br/>" +
        QString("<a href=\"https://github.com/LibreCAD/LibreCAD_3/blob/master/LICENSE\">%1</a>").arg(tr("License"))
    );

    auto copy_button = new QPushButton(tr("Copy"));
    // copy_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layout->addWidget(copy_button);

    connect(copy_button, SIGNAL(released()), this, SLOT(copyClick()));

    auto links_label = new QLabel(links);
    links_label->setOpenExternalLinks(true);
    links_label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    layout->addWidget(links_label);
}

void AboutDialog::copyClick(){
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(_info);
}
