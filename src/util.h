#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
using namespace std;

#define UNUSED(x) (void)(x)

namespace util {
    string trim(const string& str);
    string getPWD();
    void replace(string& str, const string str1, const string str2);
}

#endif // __UTIL_H__
