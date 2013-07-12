#include "pygtk_backend.hpp"

namespace pygtk_backend
{
    void PyGuiWriter::generate(const vector<symbol::Argument *> &args)
    {
        indent() << "class GUI:\n";
        indent_inc();
        indent() << "def __init__(self, args):\n";
        indent_inc();
        indent() << "self.args = args\n";
        indent() << "self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)\n";
        indent() << "self.window.connect('delete_event', self.delete)\n";
        indent() << "self.window.connect('destroy', self.destroy)\n";
        indent() << "self.window.show()\n\n";
        indent() << "self.filename = None\n\n";
        indent() << "self.vbox = gtk.VBox()\n";
        indent() << "self.vbox.show_all()\n";
    }

    void PyGtkBackend::generate(ostream &os,
                                const vector<symbol::Argument *> &args,
                                ast::Statements *body)
    {
        if (body) {
            PyGuiWriter g(os);
            g.generate(args);
        }
    }
}
