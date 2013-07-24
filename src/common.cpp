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
