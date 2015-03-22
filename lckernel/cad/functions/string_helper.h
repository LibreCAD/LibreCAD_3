#pragma once
#include <string>
#include <functional>

namespace lc {

    using namespace std;
    class string_helper {

        public:
            /**
             * @brief string formater for dim values
             *
             * @param explicitValue "coord: <>" would be coord: 10.3
             * @param format passed to sprintf
             * @param args passed to snprinf
             * @return std::string
             */
            template<typename ... Args>
            static string dim_value(string explicitValue, const string format, const Args ... args) {
                /* asume we always have "<>" somewhere */
                size_t pos = explicitValue.find("<>");
                if (pos != string::npos)
                    return explicitValue.replace(pos, 2, string_helper::string_format(format, args...));

                /* "" */
                if (explicitValue.empty())
                    return string_helper::string_format(format, args...);

                if (explicitValue == " ")
                    return "";

                return explicitValue;
            }
            /**
             * Copied the function from http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
             */
            template<typename ... Args>
            static string string_format(const std::string &format, Args ... args) {
                size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
                std::unique_ptr<char[]> buf(new char[size]);
                snprintf(buf.get(), size, format.c_str(), args ...);
                return std::string(buf.get(), buf.get() + size);
            }

    };




}
