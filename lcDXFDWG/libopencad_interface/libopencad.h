#pragma once

#include <cad/document/document.h>
#include <cad/operations/entitybuilder.h>
#include <cad/meta/layer.h>

#include <opencad_api.h>
#include <cad/operations/builder.h>

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
                void addGeometry(const lc::Layer_SPtr& layer, const CADGeometry* geometry);

                void addArc(lc::Layer_SPtr layer, const CADArc* arc);
                void addLine(lc::Layer_SPtr layer, const CADLine* line);
                void addCircle(lc::Layer_SPtr layer, const CADCircle* circle);
                void addEllipse(lc::Layer_SPtr layer, const CADEllipse* ellipse);
                void addLWPolyline(lc::Layer_SPtr layer, const CADLWPolyline* lwPolyline);

                MetaInfo_SPtr metaInfo(const CADGeometry* geometry);
                geo::Coordinate toLcPostiton(const CADVector& position);

                Document_SPtr _document;
                operation::Builder_SPtr _builder;
                operation::EntityBuilder_SPtr _entityBuilder;
        };
    }
}