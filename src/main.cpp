#include <cstring>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "ast.hpp"
#include "ast_printer.hpp"
#include "py_backend.hpp"
#include "type.hpp"

using namespace std;
using type::TypeFactory;

extern vector<symbol::Argument *> arguments;
extern ast::Statements *body;

extern void setup_symbol_table();
extern int yyparse();

extern FILE *yyin;

void usage(ostream &os, const char *cmd)
{
    os << "usage: " << cmd << " [-h] [-b BACKEND] [-o OUTFILE] [INFILE]\n";
    os << "\n";
    os << " -h, --help          show this help message and exit\n";
    os << " -b BACKEND          select the backend\n";
    os << " -o FILE             output to FILE instead of stdout\n";
    os << "\n";
    os << "Available backends\n";
    os << " py                  Python (2.7+) backend\n";
}

ostream &error(void)
{
    cerr << "error: ";
    return cerr;
}

ostream &warning(void)
{
    cerr << "warning: ";
    return cerr;
}

void generate(ostream &os, const string &backend)
{
        if (backend == "py") {
            py_backend::PyBackend b;
            b.generate(os, arguments, body);
        } else if (backend.empty()) {
            warning() << "no backend specified, defaulting to python\n";
            py_backend::PyBackend b;
            b.generate(os, arguments, body);
        } else {
            throw UnknownBackend("unknown backend '" + backend  + "'");
        }
}

int main(int argc, char **argv)
{
    string inpath = "";
    string outpath = "";
    string backend = "";
    bool print_ast = false;
    bool print_types = false;
    bool success;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(cout, argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-b")) {
            if (++i == argc) {
                usage(cerr, argv[0]);
                error() <<  "argument '-b' expects one "
                        "argument\n";
                return 1;
            }
            backend = argv[i];
        } else if (!strcmp(argv[i], "-o")) {
            if (++i == argc) {
                usage(cerr, argv[0]);
                error() << "argument '-o' expects one argument\n";
                return 1;
            }
            outpath = argv[i];
        } else if (!strcmp(argv[i], "-a")) {
            print_ast = true;
        } else if (!strcmp(argv[i], "-s")) {
            print_types = true;
        } else if (argv[i][0] == '-') {
            usage(cerr, argv[0]);
            error() << "invalid argument '"<< argv[i] << "'\n";
            return 1;
        } else {
            inpath = argv[i];
        }
    }

    if (isatty(STDIN_FILENO)) {
        /* Read data from input file */

        if (inpath.empty()) {
            usage(cerr, argv[0]);
            error() << "no input file specified\n";
            return 1;
        }

        if (!(yyin = fopen(inpath.c_str(), "r"))) {
            if (errno == ENOENT) {
                error() << "no such file or directory: '" << inpath << "'\n";
                return 1;
            } else {
                error() << "couldn't open '" << inpath << "': "
                    << strerror(errno) << "\n";
                return 1;
            }
        }
    } else {
        /* Read data from pipe / directed file */
        yyin = stdin;
    }

    /* Parse */
    setup_symbol_table();
    success = (yyparse() == 0);

    /* Print the defined types */
    if (print_types) {
        cerr << "Defined types:\n";
        TypeFactory::print(cerr);
    }

    /* Print the syntax tree */
    if (print_ast) {
        ast_printer::AST_Printer p;
        if (body)
            body->accept(p);
    }

    if (!success)
        return 1;

    try {
        if (!outpath.empty()) {
            /* Output to file */
            ofstream f(outpath);
            if (!f) {
                error() << "failed to open '" << outpath << "' for writing\n";
                return 1;
            }
            generate(f, backend);
            f.close();
        } else {
            /* Output to stdout */
            generate(cout, backend);
        }
    } catch (const UnknownBackend &e) {
        usage(cerr, argv[0]);
        error() << e.what() << endl;
        return 1;
    } catch (const BackendException &e) {
        error() << e.what() << endl;
        return 1;
    }

    return 0;
}
