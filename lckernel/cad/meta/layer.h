#pragma once

#include <string>

#include "cad/const.h"

#include "metalinewidth.h"
#include "cad/meta/color.h"


namespace lc {
    class Layer : public MetaType  {
        public:
            Layer();
            Layer(const std::string name, const MetaLineWidth lineWidth, const Color color);

            // Change this to meta data?
            Layer(const std::string name, const Color color);
            Layer(const std::string name, const MetaLineWidth lineWidth);
            virtual ~Layer();


            virtual std::string lcMetaName() const {
                return Layer::LCMETANAME();
            }
            static std::string LCMETANAME() {
                return "_LAYER";
            }

            MetaLineWidth lineWidth() const;
            Color color() const;
            std::string name() const;

            bool operator == (const Layer& layer) const {
                return (layer._name == _name);
            }

        private:
            std::string _name;
            MetaLineWidth _lineWidth;
            Color _color;

        friend std::ostream& operator<<(std::ostream& os, const Layer& layer) {
            os << "Layer(name" << layer.name() << " lineWidth=" << layer.lineWidth() << " color=" << layer.color() << ")" << std::endl;
            return os;
        }
    };
    typedef std::shared_ptr<Layer> Layer_SPtr;
    typedef std::shared_ptr<const Layer> Layer_CSPtr;
}

// LAYER_H
