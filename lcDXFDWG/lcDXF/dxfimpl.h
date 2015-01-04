#pragma once

#include "dxfrw_intf.h"

#include <cad/document/document.h>
#include <cad/document/storagemanager.h>
#include <cad/meta/icolor.h>
#include <cad/operations/builder.h>

class DXFimpl : public DRW_InterfaceImpl {
    public:
        DXFimpl(lc::Document *document, lc::operation::Builder_SPtr builder);
        virtual void addLine(const DRW_Line& data);
        virtual void addCircle(const DRW_Circle& data);
        virtual void addLayer(const DRW_Layer& data);
        virtual void addArc(const DRW_Arc& data);
        virtual void addEllipse(const DRW_Ellipse& data);

        lc::operation::Builder_SPtr _builder;
        lc::Document *_document;
    private:
        lc::iColor icol;

};
