#pragma once

#include <string>

#include "format.h"

namespace lc {
namespace persistence {

/**
 * Where a document came from, which is what Save needs to know.
 *
 * lcUI held a bare File::Type for this, which could answer only "what format
 * would I write?" -- and answered it with an uninitialised value if the user
 * ever picked a filter the type list did not contain. It could not say whether
 * the document had a file at all, whether the read of that file finished, or
 * whether the format it came from is one LibreCAD can write. Each of those
 * decides whether Save may overwrite the original.
 */
struct DocumentSource {
    bool hasPath{false};      //!< false for a drawing that has never been saved
    std::string path;
    std::string variantId;    //!< the variant it was read from, or last written as
    bool partial{false};      //!< the read stopped early; the document holds less than the file
    bool writable{false};     //!< LibreCAD can write this variant
};

/** What Save should do with a document. */
enum class SaveAction {
    WriteToPath,    //!< overwrite the file it came from, in the same variant
    AskForTarget,   //!< Save As: there is no safe file and variant to reuse
};

struct SaveDecision {
    SaveAction action{SaveAction::AskForTarget};
    std::string path;       //!< only meaningful for WriteToPath
    std::string variantId;  //!< only meaningful for WriteToPath
    std::string reason;     //!< why the user is being asked; empty for WriteToPath
};

/**
 * Decide what Save does, without any UI.
 *
 * Overwriting the source file is only right when the document is a complete
 * reading of it in a format LibreCAD can write back. Anything else asks the
 * user, and says why.
 */
SaveDecision decideSave(const DocumentSource& source);

/** The source a finished import describes. */
DocumentSource sourceFromImport(const std::string& path, const ImportResult& result);

}  // namespace persistence
}  // namespace lc
