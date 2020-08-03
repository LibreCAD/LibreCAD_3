#pragma once
#include <string>
#include <map>

class Pattern{
	int width;
	int height;
};

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
