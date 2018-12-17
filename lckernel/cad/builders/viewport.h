#pragma once
#include "cad/const.h"

namespace lc {
    namespace meta {
        class Viewport;
        DECLARE_SHORT_SHARED_PTR(Viewport)
    }

    namespace builder {
        class ViewportBuilder {
            public:
                ViewportBuilder();
                virtual ~ViewportBuilder() = default;

                const std::string& name() const;

                ViewportBuilder* setName(const std::string& name);

                meta::Viewport_CSPtr build();

            private:
                bool checkValues();

                std::string _name;
        };
    }
}