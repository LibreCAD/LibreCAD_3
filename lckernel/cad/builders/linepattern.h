#pragma once

#include <cad/const.h>
#include <string>
#include <vector>
#include <memory>

namespace lc {
    class DxfLinePatternByValue;
    DECLARE_SHORT_SHARED_PTR(DxfLinePatternByValue)

    namespace builder {
        class LinePatternBuilder {
            public:
                LinePatternBuilder();

                virtual ~LinePatternBuilder();

                const std::string& name() const;
                const std::string& description() const;
                const std::vector<double>& path() const;

                LinePatternBuilder* setName(const std::string& name);
                LinePatternBuilder* setDescription(const std::string& description);
                LinePatternBuilder* setPath(const std::vector<double>& path);
                LinePatternBuilder* addElement(double element);

                bool checkValues();
                DxfLinePatternByValue_CSPtr build();

            private:
                std::string _name;
                std::string _description;
                std::vector<double> _path;
        };
    }
}