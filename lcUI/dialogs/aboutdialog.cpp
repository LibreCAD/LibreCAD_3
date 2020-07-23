#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>

#include "aboutdialog.h"
using namespace lc::ui::dialog;

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

struct outputConfig{
	QString lineFormat;	// 1: field name, 2: value
	QString bodyFormat;	// 1: header, 2: body
	bool displayAll;	//if to display all info
}
o_htmlDisplay={"<tr><td>%1</td><td>: %2</td></tr>\n", "<b>%1</b>\n<table>%2</table>\n", false},//For display
o_html={"<tr><td>%1</td><td>: %2</td></tr>\n", "<details>\n<summary>%1</summary>\n<table>%2</table>\n</details>\n", true},
o_md={"|%1|%2|\n", "```spoiler %1\n|||\n|---|---|\n%2```\n", true},//Table without heading
o_plain={"%1: %2\n", "### %1\n%2\n", true}
;

//OS info
#if __linux__
    #include <sys/utsname.h>
    #include <stdlib.h>

QString getOsName(outputConfig& oc)
{
    struct utsname buffer;
    if (uname(&buffer) != 0) {
       return oc.lineFormat.arg("OS").arg("Linux");
    }
    QString out;
    QString machine();
    out +=
	oc.lineFormat.arg("system name").arg(buffer.sysname) + 
    	oc.lineFormat.arg("release").arg(buffer.release) + 
    	oc.lineFormat.arg("version").arg(buffer.version) +
    	oc.lineFormat.arg("machine").arg(buffer.machine);
    return out;
}
#elif _WIN32 || _WIN64
    // Not tested
    #include <windows.h>

QString getOsName(outputConfig& oc)
{
    Qstring out;

    #ifdef _WIN32
    out += oc.lineFormat.arg("Compile Arch").arg("32-bit");
    #elif _WIN64
    out += oc.lineFormat.arg("Compile Arch").arg("64-bit");
    #endif

    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (GetVersionEx(&vi) == 0) return out;
    QString platform;
    switch (vi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
        platform = "Windows 3.x";
    case VER_PLATFORM_WIN32_WINDOWS:
        platform = vi.dwMinorVersion == 0 ? "Windows 95" : "Windows 98";
    case VER_PLATFORM_WIN32_NT:
        platform = "Windows NT";
    default:
        platform = "Unknown";
    }


    out += oc.lineFormat.arg("Platform").arg(platform);
    return out;
}
#else
//From https://stackoverflow.com/questions/15580179/how-do-i-find-the-name-of-an-operating-system
QString getOsName(outputConfig& oc)
{
    #if __APPLE__ || __MACH__
    return oc.lineFormat.arg("OS").arg("Mac OSX");
    #elif __FreeBSD__
    return oc.lineFormat.arg("OS").arg("FreeBSD");
    #elif __unix || __unix__
    return oc.lineFormat.arg("OS").arg("Unix");
    #else
    return oc.lineFormat.arg("OS").arg("Other");
    #endif
}
#endif

QString getAdditionalOSInfo(outputConfig& oc){//From Qt
	return
#if QT_VERSION >= 0x050400
	oc.lineFormat.arg("System").arg(QSysInfo::prettyProductName()) + 
	oc.lineFormat.arg("Build Architecture").arg(QSysInfo::buildCpuArchitecture()) + 
	oc.lineFormat.arg("Current Architecture").arg(QSysInfo::currentCpuArchitecture()) + 
	oc.lineFormat.arg("Kernel Type").arg(QSysInfo::kernelType()) + 
	oc.lineFormat.arg("Kernel Version").arg(QSysInfo::kernelVersion()) + 
	oc.lineFormat.arg("Product Type").arg(QSysInfo::productType()) + 
	oc.lineFormat.arg("Product Version").arg(QSysInfo::productVersion())
#else
	QString()
#endif
;
}

// LCAD info
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

QString getLCADInfo(outputConfig& oc){
    return 
        oc.lineFormat.arg("Version").arg(QString("%1.%2").arg(VERSION_MAJOR).arg(VERSION_MINOR)) +
        oc.lineFormat.arg("Compiler").arg(getCompiler()) +
        oc.lineFormat.arg("Compiled on").arg(BUILD_DATE) +
        oc.lineFormat.arg("Build info").arg(BUILD_INFO)
    ;
}

//Ext info
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

QString getExtInfo(outputConfig& oc){
    QString boost_info = QString("%1.%2.%3").arg(BOOST_VERSION / 100000).arg(BOOST_VERSION / 100 % 1000).arg(BOOST_VERSION % 100);
    return
        oc.lineFormat.arg("Qt Version").arg(qVersion()) +
        oc.lineFormat.arg("Boost Version").arg(boost_info) +
        oc.lineFormat.arg("Lua Version").arg(getLuaVersion()) + 
	oc.lineFormat.arg("libdxfrw Version").arg(DRW_VERSION) + 
	oc.lineFormat.arg("OpenGL Version").arg(getGLVersion()) +
	oc.lineFormat.arg("CMake Version").arg(CMAKE_VERSION)
    ;
}

// Get formated text
QString getInfoTextFromConfig(outputConfig& oc){//Info is generated here
	QString out =
		oc.bodyFormat.arg("LibreCAD 3").arg(getLCADInfo(oc))+
		oc.bodyFormat.arg("External Dependencies").arg(getExtInfo(oc));
	if (oc.displayAll)
		out+=oc.bodyFormat.arg("OS").arg(getOsName(oc)+getAdditionalOSInfo(oc));
	return out;
}


AboutDialog::AboutDialog(QWidget* parent) :
        QDialog(parent){
    //Copied from LibreCAD_2/src/main/qc_application window,
    // and modified
    setWindowTitle(tr("About"));

    auto layout = new QVBoxLayout;
    setLayout(layout);

    auto app_info = new QLabel(getInfoTextFromConfig(o_htmlDisplay));
    // app_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // No interaction for table
    layout->addWidget(app_info);

    QString links
    (
        QString("<div><a href=\"https://github.com/LibreCAD/LibreCAD_3/graphs/contributors\">%1</a></div>").arg(tr("Contributors")) +
        QString("<div><a href=\"https://github.com/LibreCAD/LibreCAD_3/blob/master/LICENSE\">%1</a></div>").arg(tr("License")) +
        QString("<div><a href=\"https://github.com/LibreCAD/LibreCAD_3/tree/master\">%2</a></div>").arg("The Code")
    );

    auto button_all = new QWidget(this);
    auto button_layout = new QHBoxLayout;
    button_all->setLayout(button_layout);
    layout->addWidget(button_all);

    auto copy_button = new QPushButton(tr("Copy Plain Text"));
    copy_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    button_layout->addWidget(copy_button);
    connect(copy_button, SIGNAL(released()), this, SLOT(copyClickPlain()));

    copy_button = new QPushButton(tr("Copy HTML (Github)"));
    copy_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    button_layout->addWidget(copy_button);
    connect(copy_button, SIGNAL(released()), this, SLOT(copyClickHTML()));

    copy_button = new QPushButton(tr("Copy MD (Zulip)"));
    copy_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    button_layout->addWidget(copy_button);
    connect(copy_button, SIGNAL(released()), this, SLOT(copyClickMD()));

    auto links_label = new QLabel(links);
    links_label->setOpenExternalLinks(true);
    links_label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    layout->addWidget(links_label);
}

void AboutDialog::copyClickPlain(){
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(getInfoTextFromConfig(o_plain));
}

void AboutDialog::copyClickHTML(){
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(getInfoTextFromConfig(o_html));
}

void AboutDialog::copyClickMD(){
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(getInfoTextFromConfig(o_md));
}
