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

extern int yyparse(ParseContext *);

void usage(ostream &os, const char *cmd)
{
    os << "usage: " << cmd << " [-h] [-b BACKEND] [-o OUTFILE] [INFILE]\n";
    os << "\n";
    os << " -h, --help          show this help message and exit\n";
    os << " -b BACKEND          select the backend\n";
    os << " -o FILE             output to FILE instead of stdout\n";
    os << " -t, --tgp           use the tgp instead\n";
    os << "\n";
    os << "Available backends\n";
    os << " bash                Bash (4.0+) backend (unfinished)\n";
    os << " py                  Python (2.7+) backend\n";
    os << " pygtk               PyGTK backend\n";
}

Backend *get_tgl_backend(const string &str)
{
    if (str.empty() || str == "py") {
        if (str.empty())
            warning() << "no backend specified, defaulting to python\n";
        return new py_backend::PyBackend;
    } else if (str == "bash") {
        return new bash_backend::BashBackend;
    } else if (str == "pygtk") {
        return new pygtk_backend::PyGtkBackend;
    } else {
        throw UnknownBackend("unknown backend '" + str  + "'");
    }
}

TgpBackend *get_tgp_backend(const string &str)
{
    if (str.empty() || str == "py") {
        if (str.empty())
            warning() << "no backend specified, defaulting to python\n";
        return new py_backend::PyTgpBackend;
    } else if (str == "bash") {
        //return new bash_backend::BashBackend;
        // TODO
        throw UnknownBackend("unknown backend '" + str  + "'");
    } else {
        throw UnknownBackend("unknown backend '" + str  + "'");
    }
}

/** Generate a file using the .tgl backend */
void generate_tgl(ostream &os, const string &backend, ParseData *data)
{
    Backend *b = get_tgl_backend(backend);
    b->generate(os, data->arguments, data->body);
}

/** Generate a file using the .tgp backend */
void generate_tgp(ostream &os, const string &backend,
                  ParseData *tgp_data, map<string, ParseData *> tgl_data)
{
    TgpBackend *b = get_tgp_backend(backend);
    b->generate(os, tgp_data, tgl_data);
}

int main(int argc, char **argv)
{
    string inpath = "";
    string outpath = "";
    string backend = "";
    bool print_ast = false;
    bool print_types = false;
    bool success;
    bool tgp = false;

    FILE *fp;
    string name;
    ParseContext *context;

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
        } else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--tgp")) {
            tgp = true;
        } else if (argv[i][0] == '-') {
            usage(cerr, argv[0]);
            error() << "invalid argument '"<< argv[i] << "'\n";
            return 1;
        } else {
            inpath = argv[i];
        }
    }

    if (tgp || isatty(STDIN_FILENO)) {
        /* Read data from input file */

        if (inpath.empty()) {
            usage(cerr, argv[0]);
            error() << "no input file specified\n";
            return 1;
        }

        fp = load_file(inpath.c_str());
        name = inpath;
    } else {
        /* Read data from pipe / directed file */
        fp = stdin;
        name = "";
    }

    /* Create context */
    context = new ParseContext(name, fp, tgp);

    /* Parse */
    success = (yyparse(context) == 0);

    /* Print the defined types */
    if (print_types) {
        cerr << "Defined types:\n";
        TypeFactory::print(cerr);
    }

    /* Print the syntax tree */
    if (print_ast) {
        ast_printer::AST_Printer p;
        if (context->data->body)
            context->data->body->accept(p);
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

            if (tgp)
                generate_tgp(f, backend, context->data, context->parsed_files());
            else
                generate_tgl(f, backend, context->data);
            f.close();

            /* Set the file permission to 0775 */
            if (chmod(outpath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP |
                      S_IROTH | S_IXOTH) != 0) {
                warning() << "couldn't set file permission ("
                          << strerror(errno) << ")\n";
            }
        } else {
            /* Output to stdout */
            if (tgp)
                generate_tgp(cout, backend, context->data, context->parsed_files());
            else
                generate_tgl(cout, backend, context->data);
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
