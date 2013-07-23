#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

ostream &error(void);
ostream &warning(void);

FILE *load_file(const char *);

class Escaper
{
    public:
        string operator() (const string &in) {
            stringstream out;

            for (char c : in) {
                if (c == '\t')
                    out << "\\t";
                else if (c == '\n')
                    out << "\\n";
                else if (c == '\"')
                    out << "\\\"";
                else if (c == '\\')
                    out << "\\\\";
                else
                    out << c;
            }

            return out.str();
        }
};

struct TglFileData
{
    string path;
    int line;
};

#endif
