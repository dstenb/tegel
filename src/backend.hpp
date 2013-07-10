#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <stdexcept>
#include <vector>

using namespace std;

#include "ast.hpp"
#include "symbol.hpp"

class Backend
{
	public:
		virtual void generate(ostream &,
				const vector<symbol::Argument *> &,
				ast::Statements *) = 0;
};

class BackendException : public runtime_error
{
	public:
		BackendException(const string &s)
			: runtime_error(s) {}
};

#endif
