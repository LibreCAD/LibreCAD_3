//
// Created by R. van Twisk on 4/14/16.
//

#include "dxflinepattern.h"
#include <cmath>
#include <numeric>
#include "cad/functions/string_helper.h"

using namespace lc;

DxfLinePattern::DxfLinePattern(const std::string &name, const std::string &description, const std::vector<double> &path, const double length) :
        _name(name), _description(description), _path(path), _length(length) {
    assert(!StringHelper::isBlank(name) > 0 && "Name of DxfLinePattern must be given");
    // Continues has a path length of 0 assert(_path.size() > 0 && "Path length must be > 0");
}

double DxfLinePattern::calculatePathLength(const std::vector<double> &_path) {
    return std::fabs(std::accumulate(_path.begin(), _path.end(), 0.));
}

std::vector<double> DxfLinePattern::generatePattern(const std::vector<double> &dxfPattern, const double length, const double lineWidth) const {
    // DXF Linestyle Pattern is as follows
    // Parameters pattern – is a list of float values, elements > 0 are solid line segments, elements < 0 are gaps and elements = 0 are points. pattern[0] = total pattern length in drawing units
    // w need to generate them as follows:
    //double dashes[] = {50.0,  /* ink */
    //                   10.0,  /* skip */
    //                   10.0,  /* ink */
    //                   10.0   /* skip*/
    //};
    std::vector<double> dxfPat;

    if (dxfPattern.size() == 0) {
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
    for (unsigned int i = 0; i < dxfPat.size(); i++) {
        dxfPat[i] = dxfPat[i] * mul;
    }

    return dxfPat;
}

const std::vector<double> DxfLinePattern::lcPattern(double lineWidth) const {
    try {
        return _lcPatterns.at(lineWidth);
    }
    catch (std::out_of_range &e) {
        auto pattern = generatePattern(_path, _length, lineWidth);
        _lcPatterns[lineWidth] = pattern;
        return pattern;
    }
}