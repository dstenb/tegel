#ifndef __PYTHON_HELPERS_H__
#define __PYTHON_HELPERS_H__

#include <sstream>
#include <string>

using namespace std;

#include "common.hpp"
#include "type.hpp"

/* Shared helper functions and classes for the various Python backends */

namespace py_helpers
{

    /** PyWriter class
     *
     * PyWriter provides python indentation (i.e. levels of 4 spaces) to an
     * ostream
     *
     */
    class PyWriter
    {
        public:
            PyWriter(ostream &os, unsigned indentation = 0)
                : os_(os), indentation_(indentation) {}

            virtual ~PyWriter() {}
        protected:
            ostream &indent();
            ostream &unindent();

            void indent_dec();
            void indent_inc();
        private:
            ostream &os_;
            unsigned indentation_;
    };

}

#endif
