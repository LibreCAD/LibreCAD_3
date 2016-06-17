#pragma once

#include <string>

#include "cad/const.h"

#include "metalinewidth.h"
#include "dxflinepattern.h"
#include "cad/meta/color.h"


namespace lc {
    class Layer : public EntityMetaType, public DocumentMetaType {
        public:
            Layer();
            Layer(const std::string name, const MetaLineWidthByValue lineWidth, const Color color);
            Layer(const std::string name, const MetaLineWidthByValue lineWidth, const Color color, const std::shared_ptr<const DxfLinePattern> linepattern, const bool frozen);

            // Change this to meta data?
            Layer(const std::string name, const Color color);
            Layer(const std::string name, const MetaLineWidthByValue lineWidth);
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
            const std::shared_ptr<const DxfLinePattern> linePattern() const;
            bool isFrozen() const;
            virtual const std::string name() const;

            bool operator == (const Layer& layer) const {
                return (layer._name == _name);
            }

        private:
            std::string _name;
            MetaLineWidthByValue _lineWidth;
            Color _color;
            std::shared_ptr<const DxfLinePattern> _linepattern;
            bool _isFrozen;

            friend std::ostream& operator<<(std::ostream& os, const Layer& layer) {
                os << "Layer(name" << layer.name() << " lineWidth=" << layer.lineWidth() << " color=" << layer.color() << ")" << std::endl;
                return os;
            }
    };
    using Layer_SPtr = std::shared_ptr<Layer>;
    using Layer_CSPtr = std::shared_ptr<const Layer>;
}

