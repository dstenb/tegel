#ifndef __COMMON_H__
#define __COMMON_H__

#include <sstream>
#include <string>

using std::string;

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

#endif
