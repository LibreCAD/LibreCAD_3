#pragma once

#include <string>

#include <cad/geometry/geoarea.h>
#include <cad/geometry/geocoordinate.h>

namespace lc {
namespace entity {

/**
 * An estimated bounding box for a block of text.
 *
 * The kernel has no font. lcviewernoqt links lckernel, not the other way
 * round, so nothing here can measure a string -- which means this cannot be
 * the rendered extents and should not be mistaken for them. It is what the
 * quadtree indexes and what selection, snapping and zoom-to-fit ask, so it
 * needs to be roughly right and cheap; it is recomputed every time the
 * entity container is walked.
 *
 * What it does account for, and the old one-line square did not: the number
 * of lines, the alignment the block hangs from, and the rotation.
 *
 * @param insertionPoint the anchor, in world coordinates
 * @param text           the plain text, newlines and all
 * @param height         nominal glyph height
 * @param angle          rotation about the insertion point, radians
 * @param halign         lc::TextConst::HAlign
 * @param valign         lc::TextConst::VAlign
 * @param lineSpacingFactor DXF group 44; 1.0 for a TEXT, which has no such thing
 * @param multiLine      true for MTEXT: lines break and the block grows down
 */
geo::Area textBlockBoundingBox(const geo::Coordinate& insertionPoint,
                               const std::string& text,
                               double height,
                               double angle,
                               int halign,
                               int valign,
                               double lineSpacingFactor,
                               bool multiLine);

}  // namespace entity
}  // namespace lc
