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
        public:
            PyGuiWriter(ostream &os)
                : PyWriter(os, 0) {}

            void generate(const vector<symbol::Argument *> &);
        private:
            void gen_init();
            void gen_bottom();
            void gen_top();
            void gen_delete();
            void gen_destroy();
            void gen_main();
            void gen_primitive_methods();
            void gen_menu_callbacks();
    void gen_save_methods();
    };

    class PyGtkMain : public PyWriter
    {
        public:
            PyGtkMain(ostream &os)
                : PyWriter(os, 0) {}

            void generate(const vector<symbol::Argument *> &);
        private:
            void generate_opts(const vector<symbol::Argument *> &);
    };

    class PyGtkBackend : public Backend
    {
        public:
            void generate(ostream &, const vector<symbol::Argument *> &,
                          ast::Statements *);
        private:
            void check_cmd(const vector<symbol::Argument *> &);
    };
}

#endif
