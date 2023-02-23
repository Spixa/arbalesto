#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <locale>

template <typename T>
std::string toString(const T& value);

class Fmt {
public:
    template<class T>
    static std::string formatNumber(T value);
};