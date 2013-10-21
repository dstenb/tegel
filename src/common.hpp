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

/** AsciiStringCreator
 *
 * Returns a string from the list of strings
 * "{prefix}a", ..., "{prefix}z", "{prefix}aa", ... for each call to next()
 *
 */
class AsciiStringCreator
{
    public:
        AsciiStringCreator(const string &prefix)
            : prefix_(prefix), str_(prefix_ + "`") {}

        virtual ~AsciiStringCreator() {}

        string next() {
            bool append = true;

            for (auto i = str_.length() - 1; i >= prefix_.length(); i--) {
                if (str_[i] == 'z') {
                    str_[i] = 'a';
                } else {
                    str_[i]++;
                    append = false;
                    break;
                }
            }

            if (append)
                str_ += "a";

            return str_;
        }

    private:
        string prefix_;
        string str_;
};

#endif
