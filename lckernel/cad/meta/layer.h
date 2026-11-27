#pragma once

#include <string>

#include "cad/const.h"

#include "metalinewidth.h"
#include "dxflinepattern.h"
#include "cad/meta/color.h"
#include <cad/builders/layer.h>

namespace lc {
namespace meta {
class Layer : public EntityMetaType, public DocumentMetaType {
    friend class builder::LayerBuilder;

public:
    Layer(std::string name = "0",
          const MetaLineWidthByValue& lineWidth = MetaLineWidthByValue(1),
          const Color& color = Color(),
          DxfLinePatternByValue_CSPtr linepattern = nullptr,
          bool frozen = false
         );
    Layer(std::string name, const MetaLineWidthByValue& lineWidth, const Color& color,
          DxfLinePatternByValue_CSPtr linepattern, bool frozen, bool off);
    Layer(std::string name, const MetaLineWidthByValue& lineWidth, const Color& color,
          DxfLinePatternByValue_CSPtr linepattern, bool frozen, bool off, bool locked);

    virtual ~Layer() = default;

    virtual const std::string id() const {
        return Layer::LCMETANAME() + "_" + _name;
    }

    virtual const std::string metaTypeID() const {
        return Layer::LCMETANAME();
    }

    static const std::string LCMETANAME() {
        return "_LAYER";
    }

    MetaLineWidthByValue lineWidth() const;

    Color color() const;

    DxfLinePatternByValue_CSPtr linePattern() const;

    bool isFrozen() const;
    bool isOff() const;
    bool isLocked() const;
    bool isHidden() const { return _isOff || _isFrozen; }

    virtual const std::string name() const;

    bool operator==(const Layer& layer) const {
        return (layer._name == _name);
    }

private:
    Layer(const builder::LayerBuilder& builder);

    std::string _name;
    MetaLineWidthByValue _lineWidth;
    Color _color;
    DxfLinePatternByValue_CSPtr _linepattern;
    bool _isFrozen;
    bool _isOff;
    bool _isLocked;

    friend std::ostream& operator<<(std::ostream& os, const Layer& layer) {
        os << "Layer(name=" << layer.name() << " lineWidth=" << layer.lineWidth() << " color="
           << layer.color() << ")" << std::endl;
        return os;
    }
};

DECLARE_SHORT_SHARED_PTR(Layer)
}
}

