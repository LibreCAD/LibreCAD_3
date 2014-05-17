#ifndef GEOTEXT_H
#define GEOTEXT_H

#include "cad/const.h"
#include "geocoordinate.h"
#include <QString>

namespace lc {
    namespace geo {
        class Text  {
            public:
            /**
             * Vertical alignments.
             */
            enum VAlign {
            VABaseline, /**< Bottom */
            VABottom,   /**< Bottom */
            VAMiddle,   /**< Middle */
            VATop       /**< Top. */
            };

            /**
             * Horizontal alignments.
             */
            enum HAlign {
            HALeft,     /**< Left */
            HACenter,   /**< Centered */
            HARight,    /**< Right */
            HAAligned,  /**< Aligned */
            HAMiddle,   /**< Middle */
            HAFit       /**< Fit */
            };

            /**
             * Text drawing direction.
             */
            enum TextGeneration {
            None,      /**< Normal text */
            Backward,  /**< Mirrored in X */
            UpsideDown /**< Mirrored in Y */
            };

            Text(const Coordinate& insertion_point,
                 const Coordinate& second_point, const double height,
                 const QString text_value, const double width_rel,
                 const double angle, const QString style,
                 const TextGeneration textgeneration,
                 const HAlign halign, const VAlign valign);

                const Coordinate& insertion_point() const;
                double height() const;
                QString text_value() const;
                const Coordinate& second_point() const;
                double width_rel() const;
                double angle() const;
                QString style() const;
                TextGeneration textgeneration() const;
                HAlign halign() const;
                VAlign valign() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _insertion_point, _second_point;
                double _height, _width_rel, _angle;
                VAlign _valign;
                HAlign _halign;
                TextGeneration _textgeneration;
                QString _style, _text_value;

        };
    }
}

#endif // GEOTEXT_H
