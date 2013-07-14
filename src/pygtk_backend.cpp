#include "pygtk_backend.hpp"

namespace pygtk_backend
{
    void PyGuiWriter::generate(const vector<symbol::Argument *> &args)
    {
        indent() << "class GUI:\n";
        indent_inc();
        gen_init();
        gen_delete();
        gen_destroy();
        gen_main();
        gen_primitive_methods();
    }

    void PyGuiWriter::gen_init()
    {
        indent() << "def __init__(self, args):\n";
        indent_inc();
        indent() << "self.args = args\n";
        indent() << "self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)\n";
        indent() << "self.window.connect('delete_event', self.delete)\n";
        indent() << "self.window.connect('destroy', self.destroy)\n";
        indent() << "self.window.show()\n\n";
        indent() << "self.filename = None\n\n";
        indent() << "self.create_top()\n";
        indent() << "self.create_bottom()\n\n";
        indent() << "self.vbox = gtk.VBox()\n";
        indent() << "self.vbox.pack_start(self.top, expand=False)\n";
        indent() << "self.vbox.show_all()\n";
        indent() << "self.window.add(self.vbox)\n\n";
        indent_dec();
    }

    void PyGuiWriter::gen_delete()
    {
        indent() << "def delete(self, w, e, d=None):\n";
        indent() << "    return False\n\n";
    }

    void PyGuiWriter::gen_destroy()
    {
        indent() << "def destroy(self, w, d=None):\n";
        indent() << "    gt.main_quit()\n\n";
    }

    void PyGuiWriter::gen_main()
    {
        indent() << "def main(self):\n";
        indent() << "    gtk.main()\n\n";
    }

    void PyGuiWriter::gen_primitive_methods()
    {
        indent() << "def create_label(self, string):\n";
        indent_inc();
        indent() << "l = gtk.Label(string)\n";
        indent() << "l.show()\n";
        indent() << "return l\n\n";
        indent_dec();

        indent() << "def create_labeled(self, label, *widgets):\n";
        indent_inc();
        indent() << "v = gtk.VBox()\n";
        indent() << "v.pack_start(self.create_label(label), expand=False)\n";
        indent() << "for w, e in widgets:\n";
        indent_inc();
        indent() << "v.pack_start(w, expand=e)\n";
        indent() << "w.show()\n";
        indent_dec();
        indent() << "return v\n\n";
        indent_dec();

        indent() << "def create_bool(self, label, arg_name):\n";
        indent_inc();
        indent() << "b = gtk.CheckButton(label)\n";
        indent() << "b.set_active(getattr(self.args, arg_name))\n";
        indent() << "b.connect('toggled', self.bool_toggled, arg_name)\n";
        indent() << "return b\n\n";
        indent_dec();

        indent() << "def create_int(self, label, arg_name):\n";
        indent_inc();
        indent() << "a = gtk.Adjustment(getattr(self.args, arg_name),"
                 "-65535, 65535, 1)\n";
        indent() << "i = gtk.SpinButton(a)\n";
        indent() << "i.set_value(getattr(self.args, arg_name))\n";
        indent() << "i.connect('value-changed', self.int_changed, arg_name)\n";
        indent() << "return self.create_labeled(label, (i, False))\n\n";
        indent_dec();

        indent() << "def create_string(self, label, arg_name):\n";
        indent_inc();
        indent() << "e = gtk.Entry()\n";
        indent() << "e.set_text(getattr(self.args, arg_name))\n";
        indent() << "e.connect('changed', self.string_changed, arg_name)\n";
        indent() << "return self.create_labeled(label, (e, False))\n\n";
        indent_dec();

        indent() << "def bool_toggled(self, w, name):\n";
        indent_inc();
        indent() << "setattr(self.args, name, w.get_active())\n";
        indent() << "print(name + ' = ' + str(getattr(self.args, name)))\n";
        indent() << "self.update()\n\n";
        indent_dec();

        indent() << "def int_changed(self, w, name):\n";
        indent_inc();
        indent() << "setattr(self.args, name, int(w.get_value()))\n";
        indent() << "print(name + ' = ' + str(getattr(self.args, name)))\n";
        indent() << "self.update()\n\n";
        indent_dec();

        indent() << "def string_changed(self, w, name):\n";
        indent_inc();
        indent() << "setattr(self.args, name, w.get_text())\n";
        indent() << "print(name + ' = ' + getattr(self.args, name))\n";
        indent() << "self.update()\n\n";
        indent_dec();
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
