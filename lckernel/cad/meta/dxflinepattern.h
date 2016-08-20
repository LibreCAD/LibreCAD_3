//
// Created by R. van Twisk on 4/14/16.
//

#pragma once

#include "cad/interface/metatype.h"
#include "cad/const.h"
#include <cassert>
#include <vector>
#include <map>

namespace lc {
    /**
     * Represents a line style
     */
    class DxfLinePattern : public DocumentMetaType, public EntityMetaType {


    public:

        DxfLinePattern() {}

        DxfLinePattern(const std::string &_name, const std::string &_description, const std::vector<double> &_path, const double length);

        virtual ~DxfLinePattern() = default;

        /**
         * Calculates the total length of a path
         */
        static double calculatePathLength(const std::vector<double> &_path);

        const std::string description() const {
            return _description;
        }

        virtual const std::string name() const override {
            return _name;
        }

        const std::vector<double> &path() const {
            return _path;
        }

        const double length() const {
            return _length;
        }

        virtual const std::string id() const override {
            // TODO create proper ID
            return DxfLinePattern::LCMETANAME() + "_" + _name;
        }

        virtual const std::string metaTypeID() const override {
            return DxfLinePattern::LCMETANAME();
        }

        static std::string LCMETANAME() {
            return "_LINEPATTERN";
        }

        /**
         * \brief Generate new LibreCAD compatible pattern
         * \param dxfPattern Pattern from DXF file
         * \param length Length of DXF pattern
         * \param lineWidth Width of the line (for dots size)
         * \return New line pattern
         * Generate a new pattern compatible with LibreCAD from a DXF pattern.
         */
        std::vector<double> generatePattern(const std::vector<double> & dxfPattern, const double length, const double lineWidth) const;

        /**
         * \brief Get cached LibreCAD compatible pattern
         * \param lineWidth Width of the line (for dots size)
         * \return New or cached line pattern
         * Get LibreCAD compatible pattern from cache and generate it if needed.
         */
        const std::vector<double> lcPattern(double lineWidth = 1) const;

    private:
        std::string _name;
        std::string _description;
        std::vector<double> _path;

        //Might grow out of proportions if too much line sizes are rendered
        //We should find a way to remove widths unrendered for x drawing cycles
        mutable std::map<double, std::vector<double>> _lcPatterns;

        double _length;
    };
    using DxfLinePattern_SPtr = std::shared_ptr<DxfLinePattern>;
    using DxfLinePattern_CSPtr = std::shared_ptr<const DxfLinePattern>;
}
