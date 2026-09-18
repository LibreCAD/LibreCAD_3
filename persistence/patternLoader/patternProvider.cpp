#include "patternProvider.h"
#include <build_constants.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <cad/logger/logger.h>
#include <algorithm>
#include "../file.h"

using namespace lc::persistence;

PatternProvider* PatternProvider::Instance() {
    if (!instance) {
        instance = new PatternProvider;
    }

    return instance;
}

PatternProvider::PatternProvider() {
    //load all patterns path
    std::string filename;
    if(boost::filesystem::exists(PATTERN_RESOURCE_PATH))
        for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(PATTERN_RESOURCE_PATH), {})) {
            boost::filesystem::path pathObj(entry);
            if(pathObj.has_stem())
            {
                // return the stem (file name without extension) from path object
                filename =  pathObj.stem().string();
                std::transform(filename.begin(), filename.end(),filename.begin(), ::tolower);
                LOG_INFO << "Pattern:" << filename << "-" << entry << std::endl;
                _patternLocation[filename] = entry.path().string();
            }
        }
    //Create empty to load if nothing exists
    Pattern x;
    x.name = "NULL";
    _patterns["NULL"] = x;
}

#include <iostream>
void PatternProvider::loadPattern(std::string filename) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    // getAvailableLibrariesForFormat answers with an empty map for a format
    // nothing can read -- which is what "dwg" does in a default build -- so
    // begin() here was a dereference waiting for the day DXF joined it.
    const auto availableLibraries = File::getAvailableLibrariesForFormat("dxf");
    if (availableLibraries.empty()) {
        LOG_ERROR << "No reader for DXF; hatch pattern " << filename << " cannot be loaded";
        return;
    }

    lc::persistence::File::open(document, _patternLocation.at(filename),
                                availableLibraries.begin()->first);
    auto entityContainer = document->entitiesByBlock(nullptr);
    Pattern x;
    x.name = filename;
    x.boundingBox = entityContainer.boundingBox();
    x.entities = entityContainer.asVector();
    _patterns[filename] = x;
    LOG_INFO << "Pattern Loaded " << filename << std::endl;
}

const Pattern& PatternProvider::getPattern(std::string filename) {
    //Upper case it
    std::transform(filename.begin(), filename.end(),filename.begin(), ::tolower);
    try {
        auto pos = _patterns.find(filename);
        if (pos == _patterns.end()) {
            auto filePath = _patternLocation.at(filename);
            // Load the pattern
            loadPattern(filename);
            return _patterns.at(filename);
        } else {
            return pos->second;
        }
    } catch(const std::out_of_range& e) {
        // Unsupported pattern. The constructor puts "NULL" in the cache for
        // exactly this; returning begin() returned whatever happened to sort
        // first, which is some other drawing's pattern once one is loaded.
        return _patterns.at("NULL");
    }
};

PatternProvider* PatternProvider::instance = nullptr;
