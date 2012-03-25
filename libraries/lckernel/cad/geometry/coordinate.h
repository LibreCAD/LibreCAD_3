#ifndef COORDINATE_H
#define COORDINATE_H

namespace lc {
    namespace geo {
        class Coordinate {
            public:
                Coordinate() {
                    _x = _y = _z = 0;
                };
                Coordinate(double x, double y, double z);
                Coordinate(double x, double y);

                Coordinate(const Coordinate& coordinate);
                Coordinate& operator = (const Coordinate& coordinate);

                double x() const;
                double y() const;
                double z() const;

            private:
                double _x;
                double _y;
                double _z;
        };
    }
}
#endif // COORDINATE_H
