#pragma once

#include <cad/meta/metalinewidth.h>
#include <cad/meta/color.h>
#include <cad/meta/dxflinepattern.h>

namespace lc {
namespace meta {
class Layer;
DECLARE_SHORT_SHARED_PTR(Layer)
}

namespace builder {
class LayerBuilder {
public:
    LayerBuilder();
    virtual ~LayerBuilder() = default;

    const std::string& name() const;
    const meta::MetaLineWidthByValue& lineWidth() const;
    const Color& color() const;
    meta::DxfLinePatternByValue_CSPtr linePattern() const;
    bool isFrozen() const;

    LayerBuilder* setName(const std::string& name);
    LayerBuilder* setLineWidth(const meta::MetaLineWidthByValue& lineWidth);
    LayerBuilder* setColor(const Color& color);
    LayerBuilder* setLinePattern(const meta::DxfLinePatternByValue_CSPtr& linePattern);
    LayerBuilder* setIsFrozen(bool isFrozen);

    meta::Layer_CSPtr build();

private:
    bool checkValues();

    std::string _name;
    meta::MetaLineWidthByValue _lineWidth;
    Color _color;
    meta::DxfLinePatternByValue_CSPtr _linePattern;
    bool _isFrozen;
};
}
}