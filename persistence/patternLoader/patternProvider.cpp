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
    auto availableLibraries = File::getAvailableLibrariesForFormat("dxf");
    lc::persistence::File::open(document, _patternLocation.at(filename), availableLibraries.begin()->first);
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
        //Unsupported pattern, return whatever is in cache
        return _patterns.begin()->second;
    }
};

PatternProvider* PatternProvider::instance = nullptr;
