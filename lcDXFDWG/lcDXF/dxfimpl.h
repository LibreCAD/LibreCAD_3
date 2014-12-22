#pragma once

#include "dxfrw_intf.h"
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

class DXFimpl : public DRW_InterfaceImpl {
    public:
        DXFimpl(lc::StorageManager_SPtr s, lc::Document* d);
        virtual void addLine(const DRW_Line& data);
        virtual void addCircle(const DRW_Circle& data);
        lc::Document* _document;
        lc::StorageManager_SPtr _storageManager;
        lc::Document* document();
        lc::StorageManager_SPtr storageManager();
};
