#pragma once

#include <cad/storage/document.h>
#include <cad/operations/entitybuilder.h>
#include <cad/meta/layer.h>

#include <opencad_api.h>
#include <cad/operations/builder.h>

namespace lc {
    namespace persistence {
        class LibOpenCad {
            public:
                LibOpenCad(storage::Document_SPtr document, lc::operation::Builder_SPtr builder = nullptr);

                /**
                 * Open a file with libopencad
                 * @param file File to open
                 */
                void open(const std::string& file);

                void save(const std::string& file);

            private:
                meta::Layer_SPtr addLayer(const CADLayer& layer);
                void addGeometry(const lc::meta::Layer_SPtr& layer, const CADGeometry* geometry);

                void addArc(lc::meta::Layer_SPtr layer, const CADArc* arc);
                void addLine(lc::meta::Layer_SPtr layer, const CADLine* line);
                void addCircle(lc::meta::Layer_SPtr layer, const CADCircle* circle);
                void addEllipse(lc::meta::Layer_SPtr layer, const CADEllipse* ellipse);
                void addLWPolyline(lc::meta::Layer_SPtr layer, const CADLWPolyline* lwPolyline);

                meta::MetaInfo_SPtr metaInfo(const CADGeometry* geometry);
                geo::Coordinate toLcPostiton(const CADVector& position);

                storage::Document_SPtr _document;
                operation::Builder_SPtr _builder;
                operation::EntityBuilder_SPtr _entityBuilder;
        };
    }
}