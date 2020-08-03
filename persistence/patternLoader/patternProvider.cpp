#include "patternProvider.h"
#include <build_constants.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <cad/logger/logger.h>
#include <algorithm>

PatternProvider* PatternProvider::Instance() {
	if (!instance) {
		instance = new PatternProvider;
	}

	return instance;
}

PatternProvider::PatternProvider(){
	//load all patterns path
	std::string filename;
	for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(PATTERN_RESOURCE_PATH), {})){
		boost::filesystem::path pathObj(entry);
		if(pathObj.has_stem())
        	{
           		// return the stem (file name without extension) from path object
           		filename =  pathObj.stem().string();
           		std::transform(filename.begin(), filename.end(),filename.begin(), ::toupper);
           		LOG_INFO << "Pattern:" << filename << "-" << entry << std::endl;
           		_patternLocation[filename] = entry.path().string();
        	}
        }
        loadPattern(filename);//load something in cache
}

#include <iostream>
void PatternProvider::loadPattern(std::string filename){
	LOG_INFO << "Loading Pattern " << filename << std::endl;
	Pattern x;
	_patterns[filename] = x;
}

const Pattern& PatternProvider::getPattern(std::string filename) {
	//Upper case it
	std::transform(filename.begin(), filename.end(),filename.begin(), ::toupper);
	try{
		auto pos = _patterns.find(filename);
		if (pos == _patterns.end()) {
			auto filePath = _patternLocation.at(filename);
			// Load the pattern
			loadPattern(filename);
			return _patterns.at(filename);
		} else {
    			return pos->second;
    		}
	}catch(const std::out_of_range& e) {
		//Unsupported pattern, return whatever is in cache
        	return _patterns.begin()->second;
    	}
};

PatternProvider* PatternProvider::instance = nullptr;
