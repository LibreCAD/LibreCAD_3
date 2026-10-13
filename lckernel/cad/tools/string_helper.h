#pragma once
#include <string>
#include <functional>
#include <regex>
#include <cctype>

namespace lc {
namespace tools {
class StringHelper {

public:
    /**
     * @briefstd::string formatter for dim values
     *
     * @param explicitValue "coord: <>" would be coord: 10.3
     * @param format passed to sprintf
     * @param args passed to snprinf
     * @return std::string
     */
    template<typename ... Args>
    static std::string dim_value(std::string explicitValue, const std::string format, const Args ... args) {
        /* assume we always have "<>" somewhere */
        size_t pos = explicitValue.find("<>");
        if (pos != std::string::npos)
            return explicitValue.replace(pos, 2, StringHelper::string_format(format, args...));

        /* "" */
        if (explicitValue.empty())
            return StringHelper::string_format(format, args...);

        if (explicitValue == " ")
            return "";

        return explicitValue;
    }

    /**
     * Copied the function from http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
     */
    template<typename ... Args>
    static std::string string_format(const std::string& format, Args ... args) {
        size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size);
    }

    /**
        * Test if astd::string is blank, c
        */
    static bool isBlank(const std::string str) {
        std::regex r("[\\s]");
        std::string result = regex_replace(str, r, "");
        return result.length() == 0;
    }

    /**
     * Comparator for map to setup case Insensitive comparison ofstd::strings, useful in std::map
     */
    // Deliberately not derived from std::binary_function: it was deprecated in
    // C++11 and removed in C++17, and it only ever supplied the
    // first_argument_type/second_argument_type/result_type typedefs that
    // std::bind1st, std::bind2nd and std::not2 needed.  Those are gone too, and
    // nothing here uses the typedefs -- both structs are used purely for their
    // operator(), as a std::map comparator and a lexicographical_compare
    // predicate respectively.
    struct cmpCaseInsensitive {
        // case-independent (ci) compare_less binary function
        struct nocase_compare {
            bool operator()(unsigned char c1, unsigned char c2) const {
                return std::tolower(c1) < std::tolower(c2);
            }
        };

        bool operator()(const std::string& s1, const std::string& s2) const {
            return std::lexicographical_compare
                   (s1.begin(), s1.end(),   // source range
                    s2.begin(), s2.end(),   // dest range
                    nocase_compare());  // comparison
        }
    };

    /**
     * To lowercase std::string
     */
    static std::string tolower(std::string data) {
        std::transform(data.begin(), data.end(), data.begin(), ::tolower);
        return data;
    };
};
}
}
