#pragma once
#include <QWidget>

//Paper viewer support
namespace lc {
    namespace ui {
    	class LCADPaperViewerImpl: public QWidget{
            Q_OBJECT

        public:
            LCADPaperViewerImpl(QWidget* parent=0):QWidget(parent){};
    	};
    }
}