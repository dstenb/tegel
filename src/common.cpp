#include "common.hpp"

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

FILE *load_file(const char *path)
{
    FILE *fp;

    if (!(fp = fopen(path, "r"))) {
        if (errno == ENOENT) {
            error() << "no such file or directory: '" << path << "'\n";
            exit(1);
        } else {
            error() << "couldn't open '" << path << "': "
                    << strerror(errno) << "\n";
            exit(1);
        }
    }
    return fp;
}

char *xstrdup(const char *s)
{
    char *ns = strdup(s);

    if (!ns) {
        error() << "couldn't allocate data!\n";
        exit(1);
    }

    return ns;
}
