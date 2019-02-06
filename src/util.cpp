#include "util.h"

namespace util {
    string trim(const string& str) {
        if(str.length() == 0)
            return "";
        size_t first = str.find_first_not_of(" \t\n\r\a");
        if (std::string::npos == first) {
            switch(str[0]) {
                case ' ': case '\t': case '\n': case '\r': case '\a':
                    return "";
                break;
                default:
                break;
            }
            return str;
        }
    
        size_t last = str.find_last_not_of(" \t\n\r\a");
        return str.substr(first, (last - first + 1));
    }
    
    string getPWD() {
        char* pwd = getenv ("PWD");
        if(!pwd) return "";
        return pwd;
    }
    
    void replace(string& str, const string str1, const string str2) {
        size_t pos = str.find(str1);
        if(pos == string::npos) return;
        str.replace(pos, str1.length(), str2);
    }
}