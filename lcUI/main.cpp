#include <QApplication>
#include <QtCore/QDebug>
#include <QtWidgets/QStyleFactory>
#include <QSettings>
#include "cad/logger/logger.h"
#include "windowmanager.h"
#include <iostream>
#include <patternLoader/patternProvider.h>

#ifndef debug
	#define debug
#endif

int main2(int argc, char* argv[]) {
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
        QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    #endif
    
    QApplication a(argc, argv);

    LOGGER;//Init logger
    lc::persistence::PatternProvider::Instance();//Init pattern loader

    lc::ui::WindowManager::init();

    return QApplication::exec();
}

int main(int argc, char* argv[]) {
	#ifdef debug
	try{
	#endif
		main2(argc, argv);
	#ifdef debug
	}catch(const char* e){
		std::cerr << "Program terminated throwing" << std::endl;
		std::cerr << e << std::endl;
		throw(e);//trigger for debugger
	}
	#endif
}
