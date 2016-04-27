#pragma once
#include <string>
#include <functional>
#include <regex>

namespace lc {

    using namespace std;
    class StringHelper {

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
            static string string_format(const std::string &format, Args ... args) {
                size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
                std::unique_ptr<char[]> buf(new char[size]);
                snprintf(buf.get(), size, format.c_str(), args ...);
                return std::string(buf.get(), buf.get() + size);
            }

         /**
             * Test if a string is blank, c
             */
            static bool isBlank(const string str) {
                std::regex r("[\\s]");
                string result =  regex_replace(str, r, "");
                return result.length()==0;
            }

            /**
             * Comparator for map to setup case Insensetive comparison of strings, usefull in std::map
             */
            struct cmpCaseInsensetive : std::binary_function<std::string, std::string, bool>
            {
                // case-independent (ci) compare_less binary function
                struct nocase_compare : public std::binary_function<unsigned char,unsigned char,bool>
                {
                    bool operator() (const unsigned char& c1, const unsigned char& c2) const {
                        return std::tolower (c1) < std::tolower (c2);
                    }
                };
                bool operator() (const std::string & s1, const std::string & s2) const {
                    return std::lexicographical_compare
                            (s1.begin (), s1.end (),   // source range
                             s2.begin (), s2.end (),   // dest range
                             nocase_compare ());  // comparison
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
