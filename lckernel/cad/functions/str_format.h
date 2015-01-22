#include <memory>
#include <iostream>
#include <string>
#include <cstdio>


/**
* Copied the function from http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
*/
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args){
    size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size);
}