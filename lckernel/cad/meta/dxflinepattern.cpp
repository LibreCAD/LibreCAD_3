//
// Created by R. van Twisk on 4/14/16.
//

#include "dxflinepattern.h"
#include <cmath>
#include <numeric>
#include "cad/tools/string_helper.h"

using namespace lc::meta;

std::string DxfLinePattern::description() const {
    return "";
}

DxfLinePatternByValue::DxfLinePatternByValue(std::string name, std::string description, std::vector<double> path, const double length) :
        _name(std::move(name)),
        _description(std::move(description)),
        _path(std::move(path)),
        _length(length) {
    assert(!tools::StringHelper::isBlank(_name) > 0 && "Name of DxfLinePatternByValue must be given");
    // Continues has a path length of 0 assert(_path.size() > 0 && "Path length must be > 0");
}

DxfLinePatternByValue::DxfLinePatternByValue(const builder::LinePatternBuilder& builder) :
    _name(builder.name()),
    _description(builder.description()),
    _path(builder.path()),
    _length(calculatePathLength(builder.path())) {

}

double DxfLinePatternByValue::calculatePathLength(const std::vector<double>& path) {
    return std::fabs(std::accumulate(path.begin(), path.end(), 0.));
}

std::vector<double> DxfLinePatternByValue::generatePattern(const std::vector<double> &dxfPattern, double length, double lineWidth) const {
    // DXF Linestyle Pattern is as follows
    // Parameters pattern – is a list of float values, elements > 0 are solid line segments, elements < 0 are gaps and elements = 0 are points. pattern[0] = total pattern length in drawing units
    // w need to generate them as follows:
    //double dashes[] = {50.0,  /* ink */
    //                   10.0,  /* skip */
    //                   10.0,  /* ink */
    //                   10.0   /* skip*/
    //};
    std::vector<double> dxfPat;

    if (dxfPattern.empty()) {
        return dxfPat;
    }

    double last = dxfPattern.at(0);
    bool isInk = true;
    for (auto d : dxfPattern) {
        if (d == 0.) { // dot
            if (isInk) {
                dxfPat.push_back(lineWidth);
            } else {
                dxfPat.push_back(0.);
                dxfPat.push_back(lineWidth);
                isInk = !isInk;
            }
            d = last + 1;
        } else if (d < 0.) { // skip
            if (isInk) {
                dxfPat.push_back(0.);
                dxfPat.push_back(std::fabs(d));
                isInk = !isInk;
            } else {
                dxfPat.push_back(std::fabs(d));
            }
        } else { // ink
            if (isInk) {
                dxfPat.push_back(d);
            } else {
                dxfPat.push_back(0.);
                dxfPat.push_back(d);
                isInk = !isInk;
            }
        }
        last = last + std::fabs(d);
        isInk = !isInk;
    }

    // Set length for this pattern
    double mul = length / last;
    for (auto& part : dxfPat) {
        part = part * mul;
    }

    return dxfPat;
}

const std::vector<double> DxfLinePatternByValue::lcPattern(double lineWidth) const {
    try {
        return _lcPatterns.at(lineWidth);
    }
    catch (std::out_of_range &e) {
        auto pattern = generatePattern(_path, _length, lineWidth);
        _lcPatterns[lineWidth] = pattern;
        return pattern;
    }
}

const std::string DxfLinePatternByValue::name() const {
    return _name;
}

std::string DxfLinePatternByValue::description() const {
    return _description;
}

const std::vector<double>& DxfLinePatternByValue::path() const {
    return _path;
}

double DxfLinePatternByValue::length() const {
    return _length;
}

const std::string DxfLinePatternByBlock::name() const {
    return "ByBlock";
}