#pragma once

#include <cad/meta/metalinewidth.h>
#include <cad/meta/color.h>
#include <cad/meta/dxflinepattern.h>

namespace lc {
    class Layer;
    DECLARE_SHORT_SHARED_PTR(Layer)

    namespace builder {
        class LayerBuilder {
            public:
                LayerBuilder();
                virtual ~LayerBuilder() = default;

                const std::string& name() const;
                const MetaLineWidthByValue& lineWidth() const;
                const Color& color() const;
                DxfLinePatternByValue_CSPtr linePattern() const;
                bool isFrozen() const;

                LayerBuilder* setName(const std::string& name);
                LayerBuilder* setLineWidth(const MetaLineWidthByValue& lineWidth);
                LayerBuilder* setColor(const Color& color);
                LayerBuilder* setLinePattern(const DxfLinePatternByValue_CSPtr& linepattern);
                LayerBuilder* setIsFrozen(bool isFrozen);

                Layer_CSPtr build();

            private:
                bool checkValues();

                std::string _name;
                MetaLineWidthByValue _lineWidth;
                Color _color;
                lc::DxfLinePatternByValue_CSPtr _linePattern;
                bool _isFrozen;
        };
    }
}