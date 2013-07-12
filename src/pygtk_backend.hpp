#ifndef __PYGTK_BACKEND_H__
#define __PYGTK_BACKEND_H__

#include <sstream>
#include <string>

using namespace std;

#include "backend.hpp"
#include "common.hpp"
#include "py_backend.hpp"
#include "type.hpp"

namespace pygtk_backend {

    using namespace py_backend;

    /* Writes the GUI class */
    class PyGuiWriter : public PyWriter
    {

    };

    class PyGtkBackend : public Backend
    {
        public:
            void generate(ostream &, const vector<symbol::Argument *> &,
                          ast::Statements *);
        private:
    };
}

#endif
