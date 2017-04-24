#pragma once

#include <cad/document/document.h>
#include <cad/operations/builder.h>
#include <cad/meta/layer.h>

#include <opencad_api.h>

namespace lc {
    namespace FileLibs {
        class LibOpenCad {
            public:
                LibOpenCad(Document_SPtr document, lc::operation::Builder_SPtr builder = nullptr);

                /**
                 * Open a file with libopencad
                 * @param file File to open
                 */
                void open(const std::string& file);

                void save(const std::string& file);

            private:
                Layer_SPtr addLayer(const CADLayer& layer);
                void addGeometry(lc::Layer_SPtr layer, const CADGeometry* geometry);
                void addArc(lc::Layer_SPtr layer, const CADArc* arc);
                lc::MetaInfo_SPtr metaInfo(const CADGeometry* geometry);

                Document_SPtr _document;
                lc::operation::Builder_SPtr _builder;
        };
    }
}