#pragma once
#include <string>
#include <map>
#include <cad/base/cadentity.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/objects/pattern.h>

namespace lc {
namespace persistence {
typedef struct lc::objects::Pattern Pattern;

class PatternProvider {
public:
    static PatternProvider* Instance();
    const Pattern& getPattern(std::string);

private:
    PatternProvider();
    PatternProvider(PatternProvider const&)=delete;
    PatternProvider& operator=(PatternProvider const&)=delete;
    void loadPattern(std::string);
    static PatternProvider* instance;
    std::map<std::string, std::string> _patternLocation;
    std::map<std::string, Pattern> _patterns;
};

}
}
