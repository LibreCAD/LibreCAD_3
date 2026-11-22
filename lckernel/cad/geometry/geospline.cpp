#include "geospline.h"
#include <tinysplinecxx.cxx>
#include <cad/logger/logger.h>
#include <algorithm>
#include <array>
#include <limits>
#include <cmath>

using namespace lc;
using namespace geo;

namespace {
bool usableKnots(const std::vector<double>& knots, std::size_t count, int degree) {
    if (degree < 1 || static_cast<std::size_t>(degree) >= count) return false;
    const auto order = static_cast<std::size_t>(degree) + 1u;
    return knots.size() == count + order
        && std::all_of(knots.begin(), knots.end(), [](double v) { return std::isfinite(v); })
        && std::is_sorted(knots.begin(), knots.end())
        && knots[order - 1u] < knots[count]
        && std::isfinite(knots[count] - knots[order - 1u])
        && std::adjacent_find(knots.begin(), knots.end(), [](double a, double b) {
               return a < b && (!std::isfinite(b - a) || b - a < TS_KNOT_EPSILON);
           }) == knots.end();
}

using Homogeneous = std::array<double, 4>;
struct RationalPiece { std::vector<Homogeneous> points; unsigned depth; };

bool rationalSegments(const std::vector<Coordinate>& points, const std::vector<double>& knots,
                      const std::vector<double>& weights, int degree,
                      std::vector<BB_CSPtr>& result) {
    constexpr std::size_t maxSegments = 8192;
    constexpr unsigned maxDepth = 16;
    if (degree < 1 || degree > 3 || points.size() > maxSegments
        || (!weights.empty() && weights.size() != points.size())
        || !usableKnots(knots, points.size(), degree)) return false;
    try {
        std::vector<double> homogeneous;
        homogeneous.reserve(points.size() * 4u);
        double minimum[3] = {points[0].x(), points[0].y(), points[0].z()};
        double maximum[3] = {minimum[0], minimum[1], minimum[2]};
        for (std::size_t i = 0; i < points.size(); ++i) {
            const double w = weights.empty() ? 1.0 : weights[i];
            const double xyz[3] = {points[i].x(), points[i].y(), points[i].z()};
            if (!(w > 0) || !std::isfinite(w)) return false;
            for (int j = 0; j < 3; ++j) {
                if (!std::isfinite(xyz[j]) || !std::isfinite(xyz[j] * w)) return false;
                minimum[j] = std::min(minimum[j], xyz[j]);
                maximum[j] = std::max(maximum[j], xyz[j]);
                homogeneous.push_back(xyz[j] * w);
            }
            homogeneous.push_back(w);
        }
        const auto extent = std::max({maximum[0] - minimum[0], maximum[1] - minimum[1],
                                      maximum[2] - minimum[2]});
        if (!std::isfinite(extent)) return false;
        const auto tolerance = extent * 1e-4;
        tinyspline::BSpline curve(points.size(), 4, degree, tinyspline::BSpline::Type::Clamped);
        curve.setControlPoints(homogeneous);
        curve.setKnots(knots);
        const auto bezier = curve.toBeziers();
        const auto values = bezier.controlPoints();
        const auto order = static_cast<std::size_t>(degree) + 1u;
        if (values.size() % (4u * order) != 0u) return false;
        std::vector<BB_CSPtr> staged;
        for (std::size_t base = 0; base < values.size(); base += 4u * order) {
            RationalPiece initial{{}, 0};
            for (std::size_t i = 0; i < order; ++i)
                initial.points.push_back({values[base+4*i], values[base+4*i+1],
                                          values[base+4*i+2], values[base+4*i+3]});
            std::vector<RationalPiece> pending;
            pending.push_back(std::move(initial));
            while (!pending.empty()) {
                auto piece = std::move(pending.back()); pending.pop_back();
                std::vector<Coordinate> projected;
                for (const auto& h : piece.points) {
                    if (!(h[3] > 0) || !std::isfinite(h[3])) return false;
                    const Coordinate p(h[0]/h[3], h[1]/h[3], h[2]/h[3]);
                    if (!std::isfinite(p.x()) || !std::isfinite(p.y()) || !std::isfinite(p.z())) return false;
                    projected.push_back(p);
                }
                const auto start = projected.front(), end = projected.back();
                const double dx = end.x()-start.x(), dy = end.y()-start.y(), dz = end.z()-start.z();
                const double length = std::hypot(std::hypot(dx, dy), dz);
                if (!std::isfinite(length)) return false;
                bool flat = true;
                for (const auto& p : projected) {
                    const double x = p.x()-start.x(), y = p.y()-start.y(), z = p.z()-start.z();
                    const double along = length > 0 ? std::max(0.0, std::min(length,
                        x*(dx/length) + y*(dy/length) + z*(dz/length))) : 0.0;
                    const double distance = length > 0
                        ? std::hypot(std::hypot(x-along*(dx/length), y-along*(dy/length)), z-along*(dz/length))
                        : std::hypot(std::hypot(x, y), z);
                    if (!std::isfinite(distance)) return false;
                    if (distance > tolerance) { flat = false; break; }
                }
                if (flat) {
                    if (staged.size() >= maxSegments) return false;
                    const Coordinate middle(start.x()/2 + end.x()/2, start.y()/2 + end.y()/2,
                                            start.z()/2 + end.z()/2);
                    staged.push_back(std::make_shared<Bezier>(start, middle, end));
                    continue;
                }
                if (piece.depth >= maxDepth || staged.size() + pending.size() >= maxSegments) return false;
                std::vector<Homogeneous> work = piece.points, left(order), right(order);
                left[0] = work.front(); right[order-1] = work.back();
                for (std::size_t level = 1; level < order; ++level) {
                    for (std::size_t i = 0; i < order-level; ++i)
                        for (std::size_t j = 0; j < 4; ++j)
                            work[i][j] = work[i][j]/2 + work[i+1][j]/2;
                    left[level] = work[0]; right[order-1-level] = work[order-1-level];
                }
                pending.push_back({std::move(right), piece.depth + 1});
                pending.push_back({std::move(left), piece.depth + 1});
            }
        }
        if (staged.empty()) return false;
        result.swap(staged);
        return true;
    } catch (const std::runtime_error&) {
        return false;
    }
}
} // namespace

const std::vector<Coordinate> &Spline::controlPoints() const {
    return _controlPoints;
}

bool Spline::closed() const {
    return _closed;
}

short Spline::degree() const {
    return _degree;
}

const std::vector<double> &Spline::knotPoints() const {
    return _knotPoints;
}

const std::vector<Coordinate> &Spline::fitPoints() const {
    return _fitPoints;
}


double Spline::startTanX() const {
    return _sTanX;
}

double Spline::startTanY() const {
    return _sTanY;
}

double Spline::startTanZ() const {
    return _sTanZ;

}

double Spline::endTanX() const {
    return _eTanX;
}

double Spline::endTanY() const {
    return _eTanY;
}

double Spline::endTanZ() const {
    return _eTanZ;
}

double Spline::nX() const {
    return _nX;
}
double Spline::nY() const {
    return _nY;
}
double Spline::nZ() const {
    return _nZ;
}
Spline::splineflag Spline::flags() const {
    return _flags;
}
double Spline::fitTolerance() const {
    return _fitTolerance;
}

Coordinate Spline::nearestPointOnPath(const Coordinate &coord) const {
    /// @todo implement
    /// @todo fix compiler warning
    return Coordinate(coord);//Force the intersection for now
}
Coordinate Spline::nearestPointOnEntity(const Coordinate &coord) const {
    /// @todo implement
    /// @todo fix compiler warning
    return Coordinate();
}

void Spline::populateCurve() {
    _beziers.clear();
    if (((_flags & RATIONAL) != 0 || !_weights.empty())
        && rationalSegments(_controlPoints, _knotPoints, _weights, degree(), _beziers)) return;
    tinyspline::BSpline splineCurve;
    tinyspline::BSpline beziers;
    bool converted = false;

    try {
        auto nbControlPoints = _controlPoints.size();
        splineCurve = tinyspline::BSpline(nbControlPoints, 3, degree(), tinyspline::BSpline::Type::Clamped);
        //Set control points
        std::vector<tinyspline::real> ts_controlPoints;
        for (const auto& cp : _controlPoints) {
            ts_controlPoints.push_back(cp.x());
            ts_controlPoints.push_back(cp.y());
            ts_controlPoints.push_back(cp.z());
        }
        splineCurve.setControlPoints(ts_controlPoints);
        // Keep the existing fallback if the source knot profile is unsupported
        // or TinySpline cannot convert it.
        const bool ordinary = (_flags & RATIONAL) == 0 && _weights.empty();
        const bool validKnots = ordinary && usableKnots(_knotPoints, nbControlPoints, degree());
        if (validKnots) {
            try {
                auto candidate = splineCurve;
                candidate.setKnots(_knotPoints);
                beziers = candidate.toBeziers();
                splineCurve = candidate;
                converted = true;
            } catch (const std::runtime_error& e) {
                LOG_ERROR << "Unable to apply spline knots: " << e.what() << std::endl;
            }
        }
    }
    catch (std::runtime_error& e) {
        LOG_ERROR << e.what() << std::endl;
        splineCurve = tinyspline::BSpline();
    }

    if (!converted) {
        beziers = splineCurve.toBeziers();
    }

    int nbBeziers = beziers.numControlPoints() / splineCurve.order();
    int nbCoordinate = splineCurve.order() * splineCurve.dimension();

    auto controlPoints = beziers.controlPoints();

    if (splineCurve.degree() == 1) {
        // The viewer and intersection code consume quadratic/cubic Beziers.
        // A quadratic whose middle point is the midpoint is exactly a line.
        for (int i = 0; i < nbBeziers; ++i) {
            const auto j = nbCoordinate * i;
            const geo::Coordinate start(controlPoints[j], controlPoints[j+1], controlPoints[j+2]);
            const geo::Coordinate end(controlPoints[j+3], controlPoints[j+4], controlPoints[j+5]);
            const geo::Coordinate midpoint(start.x() / 2 + end.x() / 2,
                                           start.y() / 2 + end.y() / 2,
                                           start.z() / 2 + end.z() / 2);
            _beziers.push_back(std::make_shared<Bezier>(start, midpoint, end));
        }
    } else if(splineCurve.degree() == 2) {
        for (int i = 0; i < nbBeziers; i++) {
            std::vector<geo::Coordinate> bez;

            auto j = nbCoordinate * i;
            auto end = nbCoordinate * (i+1);

            while(j < end) {
                lc::geo::Coordinate cp(controlPoints[j], controlPoints[j+1], controlPoints[j+2]);
                bez.push_back(cp);
                j = j + 3;
            }
            _beziers.push_back(std::make_shared<Bezier>(bez.at(0),bez.at(1),bez.at(2)));
        }
    } else if(splineCurve.degree() == 3) {
        for (int i = 0; i < nbBeziers; i++) {
            std::vector<geo::Coordinate> bez;

            auto j = nbCoordinate * i;
            auto end = nbCoordinate * (i+1);

            while(j < end) {
                lc::geo::Coordinate cp(controlPoints[j], controlPoints[j+1], controlPoints[j+2]);
                bez.push_back(cp);

                j = j + 3;
            }

            _beziers.push_back(std::make_shared<CubicBezier>(bez.at(0),bez.at(1),bez.at(2),bez.at(3)));
        }
    }
}

const std::vector<BB_CSPtr> Spline::beziers() const {
    return _beziers;
}

void Spline::trimAtPoint(const geo::Coordinate& c) {
    /// @todo implement
}
