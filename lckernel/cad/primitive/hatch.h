#pragma once

#include <cad/meta/layer.h>

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {
    namespace entity {
        struct HatchLoop
        {
            std::vector<CADEntity_CSPtr> objList;
        };
        class Hatch : public std::enable_shared_from_this<Hatch>,
                       public CADEntity{
        public:
            Hatch(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInf=nullptr, meta::Block_CSPtr block=nullptr);

            Hatch(const Hatch_CSPtr& other, bool sameID = false);
        public:

            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

            virtual CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;

            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const override;
            virtual CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const override;

            virtual CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                    const geo::Coordinate& axis2) const override;

            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;
        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }
        private:
            std::string _name;          //Hatch pattern name
            int _solid;                 /*!< solid fill flag, code 70, solid=1, pattern=0 */
            int _associative;           /*!< associativity, code 71, associatve=1, non-assoc.=0 */
            int _hstyle;                /*!< hatch style, code 75 */
            int _hpattern;              /*!< hatch pattern type, code 76 */
            int _doubleflag;            /*!< hatch pattern double flag, code 77, double=1, single=0 */
            int _loopsnum;              /*!< namber of boundary paths (loops), code 91 */
            double _angle;              /*!< hatch pattern angle, code 52 */
            double _scale;              /*!< hatch pattern scale, code 41 */
            int _deflines;              /*!< number of pattern definition lines, code 78 */

            std::vector<std::shared_ptr<HatchLoop> > _loopList;  /*!< polyline list */
        public:
            void addLoop(std::shared_ptr<HatchLoop> lp);
            void setPatternName(std::string name){_name = name;};
            void setSolid(int solid){_solid = solid;};
//            int _associative;           /*!< associativity, code 71, associatve=1, non-assoc.=0 */
            void setHatchStyle(int hstyle){_hstyle = _hstyle;};
            void setHatchPattern(int hpattern){_hpattern = hpattern;};
//            int _doubleflag;            /*!< hatch pattern double flag, code 77, double=1, single=0 */
//            int _loopsnum;              /*!< namber of boundary paths (loops), code 91 */
            void setAngle(double angle){_angle = angle;};
            void setScale(double scale){_scale = scale;};
        };
        DECLARE_SHORT_SHARED_PTR(Hatch)
    }
}
