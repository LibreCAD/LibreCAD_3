#pragma once

namespace lc {

    struct TextConst {
        public:
            /**
            * Vertical alignments.
            */
            enum
            VAlign {
                VABaseline = 0, /**< Bottom */
                VABottom = 1, /**< Bottom */
                VAMiddle = 2, /**< Middle */
                VATop = 3       /**< Top. */
            };
            /**
            * Horizontal alignments.
            */
            enum
            HAlign {
                HALeft = 0, /**< Left */
                HACenter = 1, /**< Centered */
                HARight = 2, /**< Right */
                HAAligned = 3, /**< Aligned */
                HAMiddle = 4, /**< Middle */
                HAFit = 5       /**< Fit */
            };
            /**
            * MText drawing direction.
            */
            enum
            DrawingDirection {
                None = 0, /**< Normal text */
                Backward = 1, /**< Mirrored in X */
                UpsideDown = 2 /**< Mirrored in Y */
            };

            /**
            * Line spacing style for MTexts.
            */
            enum
            LineSpacingStyle {
                AtLeast,
                /**< Taller characters will override */
                Exact           /**< Taller characters will not override */
            };

    };
}