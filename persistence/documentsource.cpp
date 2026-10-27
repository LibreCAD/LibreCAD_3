#include "documentsource.h"

namespace lc {
namespace persistence {

SaveDecision decideSave(const DocumentSource& source) {
    SaveDecision decision;

    if (!source.hasPath || source.path.empty()) {
        decision.reason = "This drawing has not been saved yet.";
        return decision;
    }

    if (source.partial) {
        // The document holds less than the file does. Writing it back to the
        // same path replaces a drawing with a truncated copy of itself, and the
        // user has no way to notice: what is on screen looks like a drawing.
        decision.reason = "This drawing was only partly read, so saving it here "
                          "would replace the original with less than it holds.";
        return decision;
    }

    const auto* variant = formatVariantById(source.variantId);
    if (variant == nullptr || !variant->writable) {
        decision.reason = "LibreCAD cannot write the format this drawing came from.";
        return decision;
    }

    decision.action = SaveAction::WriteToPath;
    decision.path = source.path;
    decision.variantId = source.variantId;

    return decision;
}

DocumentSource sourceFromImport(const std::string& path, const ImportResult& result) {
    DocumentSource source;
    source.hasPath = !path.empty();
    source.path = path;
    source.variantId = result.variantId;
    // A read that failed without delivering anything leaves no drawing to save,
    // but it also must not be treated as a complete reading of the file.
    source.partial = result.partial || !result.ok;

    const auto* variant = formatVariantById(result.variantId);
    source.writable = variant != nullptr && variant->writable;

    return source;
}

}  // namespace persistence
}  // namespace lc
