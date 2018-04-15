#pragma once

#include <string>

#include "cad/const.h"

#include "metalinewidth.h"
#include "dxflinepattern.h"
#include "cad/meta/color.h"
#include <cad/builders/layer.h>

namespace lc {
    class Layer : public EntityMetaType, public DocumentMetaType {
        friend class builder::LayerBuilder;

        public:
            Layer(std::string name = "0",
                  MetaLineWidthByValue lineWidth = MetaLineWidthByValue(1),
                  Color color = Color(),
                  DxfLinePatternByValue_CSPtr linepattern = nullptr,
                  bool frozen = false
            );

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
            virtual const std::string name() const;

            bool operator == (const Layer& layer) const {
                return (layer._name == _name);
            }

        private:
            Layer(const builder::LayerBuilder& builder);

            std::string _name;
            MetaLineWidthByValue _lineWidth;
            Color _color;
            DxfLinePatternByValue_CSPtr _linepattern;
            bool _isFrozen;

            friend std::ostream& operator<<(std::ostream& os, const Layer& layer) {
                os << "Layer(name" << layer.name() << " lineWidth=" << layer.lineWidth() << " color=" << layer.color() << ")" << std::endl;
                return os;
            }
    };

    DECLARE_SHORT_SHARED_PTR(Layer)
}

