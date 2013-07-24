#include <cstring>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <sys/stat.h>

#include "ast.hpp"
#include "ast_printer.hpp"
#include "data.hpp"
#include "type.hpp"

#include "bash_backend.hpp"
#include "py_backend.hpp"
#include "pygtk_backend.hpp"

using namespace std;
using type::TypeFactory;

extern ParseData *yydata;

extern vector<TglFileData> tgl_files;
extern bool tgp_file;

extern void setup_symbol_table();
extern int yyparse();
extern void yysetin(FILE *);

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
    os << " bash                Bash (4.0+) backend (unfinished)\n";
    os << " py                  Python (2.7+) backend\n";
    os << " pygtk               PyGTK backend\n";
}

void generate(ostream &os, const string &backend,
        ParseData *tgp_data, map<string, ParseData *> tgl_data)
{
    if (backend == "bash") {
        // TODO
        //bash_backend::BashBackend b;
        //b.generate(os, tgp_data, tgl_data);
    } else if (backend.empty() || backend == "py") {
        if (backend.empty())
            warning() << "no backend specified, defaulting to python\n";
        py_backend::PyTgpBackend b;
        b.generate(os, tgp_data, tgl_data);
    } else if (backend == "pygtk") {
        // TODO
        //pygtk_backend::PyGtkBackend b;
        //b.generate(os, yydata->arguments, yydata->body);
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

    ParseData *tgp_data;
    map<string, ParseData *> tgl_data;

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

    if (inpath.empty()) {
        usage(cerr, argv[0]);
        error() << "no input file specified\n";
        return 1;
    }

    yyin = load_file(inpath.c_str());

    /* Parse */
    tgp_file = true;
    yydata = new ParseData;
    success = (yyparse() == 0);
    tgp_data = yydata;

    /* The rest of the files will be treated as .tgl files */
    tgp_file = false;

    for (auto f : tgl_files) {
        /* TODO: check and store absolute paths to avoid duplicates */
        if (tgl_data.find(f.path) == tgl_data.end()) {
            cerr << "Parsing " << f.path << endl;

            yysetin(load_file(f.path.c_str()));
            yydata = new ParseData;
            if (yyparse() != 0)
                return 1;

            tgl_data[f.path] = yydata;
        }

    }

    /* Print the defined types */
    if (print_types) {
        cerr << "Defined types:\n";
        TypeFactory::print(cerr);
    }

    /* Print the syntax tree */
    if (print_ast) {
        ast_printer::AST_Printer p;
        if (yydata->body)
            yydata->body->accept(p);
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
            generate(f, backend, tgp_data, tgl_data);
            f.close();

            /* Set the file permission to 0775 */
            if (chmod(outpath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP |
                      S_IROTH | S_IXOTH) != 0) {
                warning() << "couldn't set file permission ("
                          << strerror(errno) << ")\n";
            }
        } else {
            /* Output to stdout */
            generate(cout, backend, tgp_data, tgl_data);
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
