#pragma once

#include <cad/interface/metatype.h>
#include <cad/builders/viewport.h>

#define VIEWPORT_ID_PREFIX "VIEWPORT"

namespace lc {
    namespace meta {
        class Viewport : public DocumentMetaType {
            friend class builder::ViewportBuilder;

            public:
                Viewport(std::string name="MODEL");//Default name for model view

                const std::string id() const override;

                const std::string name() const override;

                static const std::string LCMETANAME() {
                    return "_VIEWPORT";
                }

                bool operator==(const Viewport& viewport){
                    return _name==viewport._name;
                }

            private:
                Viewport(const builder::ViewportBuilder& builder);
                std::string _name;
        };

        DECLARE_SHORT_SHARED_PTR(Viewport)
    }
}