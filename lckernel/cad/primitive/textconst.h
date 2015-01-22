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
            * Attachment Point used for dimensions
            */
            enum AttachmentPoint {
                Top_left = 1,
                Top_center = 2,
                Top_right = 4,
                Middle_left = 8 + 1,
                Middle_center = 8 + 2,
                Middle_right = 8 + 4,
                Bottom_left = 16 + 1,
                Bottom_center = 16 + 2,
                Bottom_right = 16 + 4
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