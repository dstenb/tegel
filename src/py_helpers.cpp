#include "py_helpers.hpp"

namespace py_helpers
{

    ostream &PyWriter::indent()
    {
        for (unsigned i = 0; i < indentation_; i++)
            os_ << "    ";
        return os_;
    }

    ostream &PyWriter::unindent()
    {
        return os_;
    }

    void PyWriter::indent_dec()
    {
        if (indentation_ > 0)
            indentation_--;
    }

    void PyWriter::indent_inc()
    {
        indentation_++;
    }
}
