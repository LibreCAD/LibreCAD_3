#pragma once

#include "cad/document/document.h"
#include <cad/document/storagemanager.h>
#include "cad/dochelpers/documentimpl.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/text.h"
#include "cad/primitive/coordinate.h"
#include <cad/operations/builder.h>
#include <cad/dochelpers/storagemanagerimpl.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <dwg.h>
#include <dwg_api.h>
#ifdef __cplusplus
}
#endif

class DWGimpl {
        Dwg_Data data;
    public:
        DWGimpl(lc::StorageManager_SPtr s, lc::Document* d);
        void addLine(dwg_object* obj);
        void addCircle(dwg_object* obj);
        void addArc(dwg_object* obj);
        void addText(dwg_object* obj);
        void output_BLOCK_HEADER(dwg_object_ref* ref);
        void ReadDWG(dwg_data* dwg) ;
        void OutputObject(dwg_object* obj);
        int readFile(char* filename);
        std::shared_ptr<lc::Document> _document;
        lc::StorageManager_SPtr _storageManager;
        lc::Document* document();
        lc::StorageManager_SPtr storageManager();
};
