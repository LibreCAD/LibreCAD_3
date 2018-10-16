//
// Created by R. van Twisk on 4/14/16.
//

#pragma once

#include "cad/interface/metatype.h"
#include "cad/const.h"
#include <cassert>
#include <vector>
#include <map>
#include <cad/builders/linepattern.h>

namespace lc {
    namespace meta {
        /**
         * Represents a line style
         */
        class DxfLinePattern : public DocumentMetaType, public EntityMetaType {
            public:
                virtual const std::string metaTypeID() const override {
                    return DxfLinePattern::LCMETANAME();
                }

                static std::string LCMETANAME() {
                    return "_LINEPATTERN";
                }

                const std::string id() const override {
                    return DxfLinePattern::LCMETANAME() + "_" + name();
                }

                virtual std::string description() const;
        };

        class DxfLinePatternByValue : public DxfLinePattern {
                friend class builder::LinePatternBuilder;

            public:
                DxfLinePatternByValue(std::string name = "0",
                                      std::string description = "",
                                      std::vector<double> path = {},
                                      double length = 0);

                virtual ~DxfLinePatternByValue() = default;

                /**
                 * Calculates the total length of a path
                 */
                static double calculatePathLength(const std::vector<double>& path);

                const std::string name() const override;

                std::string description() const override;

                const std::vector<double>& path() const;

                double length() const;

                /**
                 * \brief Generate new LibreCAD compatible pattern
                 * \param dxfPattern Pattern from DXF file
                 * \param length Length of DXF pattern
                 * \param lineWidth Width of the line (for dots size)
                 * \return New line pattern
                 * Generate a new pattern compatible with LibreCAD from a DXF pattern.
                 */
                std::vector<double>
                generatePattern(const std::vector<double>& dxfPattern, double length, double lineWidth) const;

                /**
                 * \brief Get cached LibreCAD compatible pattern
                 * \param lineWidth Width of the line (for dots size)
                 * \return New or cached line pattern
                 * Get LibreCAD compatible pattern from cache and generate it if needed.
                 */
                const std::vector<double> lcPattern(double lineWidth = 1) const;

            private:
                DxfLinePatternByValue(const builder::LinePatternBuilder& builder);

                std::string _name;
                std::string _description;
                std::vector<double> _path;

                //Might grow out of proportions if too much line sizes are rendered
                //We should find a way to remove widths unrendered for x drawing cycles
                mutable std::map<double, std::vector<double>> _lcPatterns;

                double _length;
        };

        class DxfLinePatternByBlock : public DxfLinePattern {
            public:
                const std::string name() const override;
        };

        DECLARE_SHORT_SHARED_PTR(DxfLinePattern)
        DECLARE_SHORT_SHARED_PTR(DxfLinePatternByValue)
        DECLARE_SHORT_SHARED_PTR(DxfLinePatternByBlock)
    }
}
