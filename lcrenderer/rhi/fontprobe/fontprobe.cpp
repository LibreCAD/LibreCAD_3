// R0 CPU-only FreeType audit, not a painter, shaper, atlas or graphics test.
#include <ft2build.h>
#include FT_FREETYPE_H
#include "font_data.h"
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::vector<std::string> checks;
void require(bool condition, const std::string& id) {
    if (!condition) throw std::runtime_error(id);
    checks.push_back(id);
}
void ftcheck(FT_Error error, const std::string& id) {
    if (error) throw std::runtime_error(id + ": FreeType error " + std::to_string(error));
    checks.push_back(id);
}
struct Library {
    FT_Library value = nullptr;
    ~Library() { if (value) FT_Done_FreeType(value); }
};
struct Face {
    FT_Face value = nullptr;
    ~Face() { if (value) FT_Done_Face(value); }
};
std::string hex(const std::vector<unsigned char>& pixels) {
    std::ostringstream out;
    for (auto pixel : pixels) out << std::hex << std::setw(2) << std::setfill('0') << unsigned(pixel);
    return out.str();
}
struct Observation {
    Probe probe;
    unsigned glyph, ppem, width, rows;
    long rawAdvance, rawLeft, scaledAdvance;
    int bitmapLeft, bitmapTop;
    std::vector<unsigned char> pixels;
    std::string snapshot;
};
}

int main(int argc, char**) {
    try {
        require(argc == 1, "no-runtime-font-or-script-inputs");
        Library library;
        ftcheck(FT_Init_FreeType(&library.value), "library-init");
        int major, minor, patch;
        FT_Library_Version(library.value, &major, &minor, &patch);
        if (major != FREETYPE_MAJOR || minor != FREETYPE_MINOR || patch != FREETYPE_PATCH)
            throw std::runtime_error("compile-runtime-provider-version: headers=" + std::to_string(FREETYPE_MAJOR) + "." +
                std::to_string(FREETYPE_MINOR) + "." + std::to_string(FREETYPE_PATCH) + " runtime=" +
                std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch));
        require(true, "compile-runtime-provider-version");
        {
            const unsigned char invalid[] = {0, 1, 2, 3, 4};
            Face rejected;
            require(FT_New_Memory_Face(library.value, invalid, sizeof(invalid), 0, &rejected.value) != 0,
                    "corrupt-font-rejected");
        }
        Face face;
        ftcheck(FT_New_Memory_Face(library.value, fontBytes, sizeof(fontBytes), 0, &face.value), "pinned-face-open");
        require(face.value->family_name && std::string(face.value->family_name) == family, "font-family");
        require(face.value->style_name && std::string(face.value->style_name) == style, "font-style");
        require(FT_IS_SCALABLE(face.value) && !FT_IS_TRICKY(face.value), "declared-scalable-font-domain");
        require(face.value->units_per_EM == unitsPerEm, "design-unit-scale");
        const std::string observedFamily = face.value->family_name;
        const std::string observedStyle = face.value->style_name;
        const auto observedUnits = face.value->units_per_EM;
        ftcheck(FT_Select_Charmap(face.value, FT_ENCODING_UNICODE), "unicode-charmap");
        std::vector<Observation> observations;
        for (const auto& probe : probes) {
            std::string prefix = probe.name;
            auto glyph = FT_Get_Char_Index(face.value, probe.codepoint);
            require(glyph == probe.glyph, prefix + ":cmap-glyph");
            ftcheck(FT_Load_Glyph(face.value, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP),
                    prefix + ":raw-glyph-load");
            const auto metrics = face.value->glyph->metrics; // Copy before slot reuse.
            require(metrics.horiAdvance == probe.advance, prefix + ":design-advance");
            if (prefix == "ordinary") require(metrics.horiBearingX == leftBearing, prefix + ":design-left-bearing");
            for (unsigned ppem : {32u, 64u}) {
                auto id = prefix + ":" + std::to_string(ppem);
                ftcheck(FT_Set_Pixel_Sizes(face.value, 0, ppem), id + ":size");
                ftcheck(FT_Load_Glyph(face.value, glyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP), id + ":scaled-load");
                auto scaledAdvance = face.value->glyph->advance.x;
                require(std::abs(scaledAdvance / 64.0 - probe.advance * double(ppem) / unitsPerEm) <= 1.0 / 64.0,
                        id + ":unhinted-advance");
                ftcheck(FT_Render_Glyph(face.value->glyph, FT_RENDER_MODE_NORMAL), id + ":rasterize");
                const auto& bitmap = face.value->glyph->bitmap;
                require(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && bitmap.num_grays == 256 && bitmap.buffer &&
                        bitmap.width > 0 && bitmap.width <= 1024 && bitmap.rows > 0 && bitmap.rows <= 1024 &&
                        bitmap.pitch >= int(bitmap.width) && bitmap.pitch <= 4096, id + ":bounded-gray-bitmap");
                Observation row{probe, glyph, ppem, bitmap.width, bitmap.rows, metrics.horiAdvance,
                                metrics.horiBearingX, scaledAdvance, face.value->glyph->bitmap_left,
                                face.value->glyph->bitmap_top, {}, {}};
                for (unsigned y = 0; y < bitmap.rows; ++y) {
                    const auto* start = bitmap.buffer + std::size_t(y) * unsigned(bitmap.pitch);
                    row.pixels.insert(row.pixels.end(), start, start + bitmap.width);
                }
                bool ink = false;
                for (auto alpha : row.pixels) ink = ink || alpha != 0;
                require(ink, id + ":nonempty-glyph-ink");
                row.snapshot = hex(row.pixels);
                observations.push_back(std::move(row));
            }
        }
        auto error = FT_Done_Face(face.value);
        face.value = nullptr;
        ftcheck(error, "face-release");
        error = FT_Done_FreeType(library.value);
        library.value = nullptr;
        ftcheck(error, "library-release");
        for (const auto& row : observations)
            require(hex(row.pixels) == row.snapshot, std::string(row.probe.name) + ":" +
                    std::to_string(row.ppem) + ":owned-pixels-after-provider-release");
        std::cout << "{\"schema_version\":1,\"evidence_kind\":\"r0-font-probe\","
                     "\"graphics_execution\":false,\"production_painter\":false,\"font_sha256\":"
                  << std::quoted(fontSha) << ",\"inputs_sha256\":" << std::quoted(inputsSha)
                  << ",\"family\":" << std::quoted(observedFamily) << ",\"style\":" << std::quoted(observedStyle)
                  << ",\"units_per_em\":" << observedUnits
                  << ",\"freetype_version\":[" << major << ',' << minor << ',' << patch
                  << "],\"observations\":[";
        bool first = true;
        for (const auto& row : observations) {
            if (!first) std::cout << ',';
            first = false;
            std::cout << "{\"variant\":" << std::quoted(row.probe.name) << ",\"codepoint\":" << row.probe.codepoint
                      << ",\"glyph\":" << row.glyph << ",\"raw_advance\":" << row.rawAdvance
                      << ",\"raw_left_bearing\":" << row.rawLeft << ",\"ppem\":" << row.ppem
                      << ",\"scaled_advance_26_6\":" << row.scaledAdvance << ",\"width\":" << row.width
                      << ",\"rows\":" << row.rows << ",\"left\":" << row.bitmapLeft << ",\"top\":" << row.bitmapTop
                      << ",\"alpha_hex\":" << std::quoted(row.snapshot) << '}';
        }
        std::cout << "],\"checks\":[";
        first = true;
        for (const auto& id : checks) {
            if (!first) std::cout << ',';
            first = false;
            std::cout << std::quoted(id);
        }
        std::cout << "]}\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "R0 font probe: FAIL: " << error.what() << '\n';
        return 1;
    }
}
