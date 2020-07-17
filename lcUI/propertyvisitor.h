#include <cad/interface/entitydispatch.h>
#include <cad/operations/entitybuilder.h>
#include <kaguya/kaguya.hpp>
#include <string>

namespace lc
{
    namespace ui
    {
        class PropertyVisitor : public lc::EntityDispatch
        {
        public:
            void visit(entity::Line_CSPtr) override;
            void visit(entity::Point_CSPtr) override;
            void visit(entity::Circle_CSPtr) override;
            void visit(entity::Arc_CSPtr) override;
            void visit(entity::Ellipse_CSPtr) override;
            void visit(entity::Text_CSPtr) override;
            void visit(entity::Spline_CSPtr) override;
            void visit(entity::DimAligned_CSPtr) override;
            void visit(entity::DimAngular_CSPtr) override;
            void visit(entity::DimDiametric_CSPtr) override;
            void visit(entity::DimLinear_CSPtr) override;
            void visit(entity::DimRadial_CSPtr) override;
            void visit(entity::LWPolyline_CSPtr) override;
            void visit(entity::Image_CSPtr) override;
            void visit(entity::Hatch_CSPtr) override;

            void setPropertyKey(const std::string& key);

            void setPropertyTable(kaguya::LuaRef propTable);

            void setEntityBuilder(std::shared_ptr<lc::operation::EntityBuilder> entBuilder);

        private:
            std::string propertyKey;
            std::string entityInfo;
            std::shared_ptr<lc::operation::EntityBuilder> entityBuilder;
            kaguya::LuaRef propertiesTable;
        };
    }
}
