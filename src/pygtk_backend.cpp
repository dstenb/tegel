#include "pygtk_backend.hpp"

namespace pygtk_backend
{
    void PyGuiWriter::generate(const vector<symbol::Argument *> &args)
    {
        indent() << "class GUI:\n";
        indent_inc();
        gen_init(args);
        gen_update();
        gen_delete();
        gen_destroy();
        gen_save_methods();
        gen_menu_callbacks();
        gen_main();
        gen_primitive_methods();
    }

    void PyGuiWriter::gen_init(const vector<symbol::Argument *> &args)
    {
        indent() << "def __init__(self, args):\n";
        indent_inc();
        indent() << "self.args = args\n";
        indent() << "self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)\n";
        indent() << "self.window.connect('delete_event', self.delete)\n";
        indent() << "self.window.connect('destroy', self.destroy)\n";
        indent() << "self.window.show()\n\n";
        indent() << "self.filename = None\n\n";
        gen_top();
        gen_bottom(args);
        indent() << "self.vbox = gtk.VBox()\n";
        indent() << "self.vbox.pack_start(self.top, expand=False)\n";
        indent() << "self.vbox.add(self.bottom)\n";
        indent() << "self.vbox.show()\n";
        indent() << "self.window.add(self.vbox)\n\n";
        indent() << "if self.args._preview:\n";
        indent() << "    self.update()\n\n";
        indent_dec();
    }

    void PyGuiWriter::gen_bottom(const vector<symbol::Argument *> &args)
    {
        indent() << "self.bottom = gtk.HBox()\n";
        indent() << "self.a_scrolled = gtk.ScrolledWindow()\n";
        indent() << "self.a_scrolled.set_policy(gtk.POLICY_AUTOMATIC, "
                 "gtk.POLICY_ALWAYS)\n";
        indent() << "self.argument_box = gtk.VBox()\n";
        indent() << "self.a_scrolled.add_with_viewport(self.argument_box)\n";
        indent() << "self.argument_box.show()\n";

        indent() << "self.p_scrolled = gtk.ScrolledWindow()\n";
        indent() << "self.p_scrolled.set_policy(gtk.POLICY_AUTOMATIC, "
                 "gtk.POLICY_ALWAYS)\n\n";
        indent() << "self.preview = gtk.TextView()\n";
        indent() << "self.preview.set_editable(False)\n";
        indent() << "self.preview.show()\n";
        indent() << "self.p_scrolled.add_with_viewport(self.preview)\n\n";

        indent() << "self.a_scrolled.show()\n";
        indent() << "self.bottom.add(self.a_scrolled)\n";
        indent() << "self.p_scrolled.set_visible(self.args._preview)\n";
        indent() << "self.bottom.add(self.p_scrolled)\n";
        indent() << "self.bottom.show()\n\n";

        //PyGuiArgumentGenerator g(unindent(), 2);
        //
        indent() << "for o in [\n";
        for (symbol::Argument *a : args) {

        }
        /* FIXME: remove dummy code */
        indent() << "self.create_bool('Bool', 'b'),\n";
        indent() << "self.create_int('Integer', 'h'),\n";
        indent() << "self.create_string('String', 's')\n";
        indent() << "]:\n";
        indent() << "    self.argument_box.pack_start(o, expand=False)\n";
        indent() << "    o.show()\n\n";

        /* TODO: Generate arguments */
    }

    void PyGuiWriter::gen_top()
    {
        indent() << "self.top = gtk.MenuBar()\n";
        indent() << "accelg = gtk.AccelGroup()\n";
        indent() << "self.window.add_accel_group(accelg)\n";

        indent() << "filemenu = gtk.Menu()\n";
        indent() << "file = gtk.MenuItem('File')\n";
        indent() << "file.set_submenu(filemenu)\n";

        indent() << "save = gtk.ImageMenuItem(gtk.STOCK_SAVE, accelg)\n";
        indent() << "save.connect('activate', self.save_callback)\n";
        indent() << "k, m = gtk.accelerator_parse('<Control>S')\n";
        indent() << "save.add_accelerator('activate', accelg, k, m, "
                 "gtk.ACCEL_VISIBLE)\n";

        indent() << "save_as = gtk.ImageMenuItem(gtk.STOCK_SAVE_AS, accelg)\n";
        indent() << "save_as.connect('activate', self.save_as_callback)\n";
        indent() << "k, m = gtk.accelerator_parse('<Shift><Control>S')\n";
        indent() << "save_as.add_accelerator('activate', accelg, k, m, "
                 "gtk.ACCEL_VISIBLE)\n";

        indent() << "exit = gtk.ImageMenuItem(gtk.STOCK_QUIT, accelg)\n";
        indent() << "exit.connect('activate', self.destroy)\n";
        indent() << "k, m = gtk.accelerator_parse('<Control>Q')\n";
        indent() << "exit.add_accelerator('activate', accelg, k, m, "
                 "gtk.ACCEL_VISIBLE)\n";

        indent() << "for o in [ save, save_as, gtk.SeparatorMenuItem(), "
                 "exit ]:\n";
        indent() << "    filemenu.append(o)\n";

        indent() << "viewmenu = gtk.Menu()\n";
        indent() << "view = gtk.MenuItem('View')\n";
        indent() << "view.set_submenu(viewmenu)\n";

        indent() << "preview = gtk.CheckMenuItem('Preview')\n";
        indent() << "preview.set_active(self.args._preview)\n";
        indent() << "preview.connect('activate', self.toggle_preview)\n";
        indent() << "viewmenu.append(preview)\n";
        indent() << "k, m = gtk.accelerator_parse('<Control>P')\n";
        indent() << "preview.add_accelerator('activate', accelg, k, m, "
                 "gtk.ACCEL_VISIBLE)\n";

        indent() << "self.top.append(file)\n";
        indent() << "self.top.append(view)\n";

        indent() << "self.top.show_all()\n";
    }

    void PyGuiWriter::gen_update()
    {
        indent() << "def update(self):\n";
        indent() << "    out = cStringIO.StringIO()\n";
        indent() << "    generate(self.args, out)\n";
        indent() << "    self.preview.get_buffer().set_text(out.getvalue())\n";
    }

    void PyGuiWriter::gen_delete()
    {
        indent() << "def delete(self, w, e, d=None):\n";
        indent() << "    return False\n\n";
    }

    void PyGuiWriter::gen_destroy()
    {
        indent() << "def destroy(self, w, d=None):\n";
        indent() << "    gtk.main_quit()\n\n";
    }

    void PyGuiWriter::gen_main()
    {
        indent() << "def main(self):\n";
        indent() << "    gtk.main()\n\n";
    }

    void PyGuiWriter::gen_save_methods()
    {
        indent() << "def save_dialog(self):\n";
        indent_inc();
        indent() << "dialog = gtk.FileChooserDialog('Save', None, "
                 "gtk.FILE_CHOOSER_ACTION_SAVE, (gtk.STOCK_CANCEL, "
                 "gtk.RESPONSE_CANCEL, gtk.STOCK_SAVE, gtk.RESPONSE_ACCEPT))\n";
        indent() << "dialog.set_do_overwrite_confirmation(True)\n";
        indent() << "dialog.set_modal(True)\n";
        indent() << "dialog.connect('response', self.save_dialog_response)\n";
        indent() << "if self.filename:\n";
        indent() << "    dialog.set_filename(self.filename)\n";
        indent() << "else:\n";
        indent() << "    dialog.set_current_name('Untitled')\n";
        indent() << "dialog.run()\n\n";
        indent_dec();

        indent() << "def save_dialog_response(self, dialog, response):\n";
        indent() << "    if response == gtk.RESPONSE_ACCEPT:\n";
        indent() << "        self.filename = dialog.get_filename()\n";
        indent() << "        self.write_to_file()\n";
        indent() << "    dialog.destroy()\n\n";

        indent() << "def write_to_file(self):\n";
        indent() << "    print('Writing to ' + self.filename)\n\n";
    }

    void PyGuiWriter::gen_menu_callbacks()
    {
        /* Preview toggle button callback */
        indent() << "def toggle_preview(self, w, data=None):\n";
        indent() << "    self.p_scrolled.set_visible(not "
                 "self.p_scrolled.get_visible())\n\n";

        /* "save" callback */
        indent() << "def save_callback(self, w, data=None):\n";
        indent() << "    if self.filename:\n";
        indent() << "        self.write_to_file()\n";
        indent() << "    else:\n";
        indent() << "        self.save_dialog()\n\n";

        /* "save as" callback */
        indent() << "def save_as_callback(self, w, data=None):\n";
        indent() << "    self.save_dialog()\n\n";
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
        indent() << "b = gtk.CheckButton(None)\n";
        indent() << "b.set_active(getattr(self.args, arg_name))\n";
        indent() << "b.connect('toggled', self.bool_toggled, arg_name)\n";
        indent() << "return self.create_labeled(label, (b, False))\n\n";
        indent_dec();

        indent() << "def create_int(self, label, arg_name):\n";
        indent_inc();
        indent() << "a = gtk.Adjustment(getattr(self.args, arg_name), "
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

    void PyGtkMain::generate(const vector<symbol::Argument *> &args)
    {
        /* TODO */
        indent() << "import gobject\n";
        indent() << "import pygtk\n";
        indent() << "pygtk.require('2.0')\n";
        indent() << "import gtk\n";
        indent() << "import cStringIO\n\n";
        indent() << "def main(argv=None):\n";
        indent_inc();
        indent() << "if argv is None:\n";
        indent() << "   argv = sys.argv\n\n";
        indent() << "cmd = argv[0]\n\n";

        generate_opts(args);
        unindent() << "\n";

        indent() << "g = GUI(args)\n";
        indent() << "g.main()\n";

        indent_dec();

        indent() << "if __name__ == \"__main__\":\n";
        indent() << "    main()\n";
    }

    void PyGtkMain::generate_opts(const vector<symbol::Argument *> &args)
    {
        indent() << "parser = argparse.ArgumentParser(description="
                 "\"Generated by TeGeL.\")\n";

        /* Add "output file" argument */
        indent() << "parser.add_argument('-o', type=argparse.FileType('w'), "
                 "default=sys.stdout, help='output to file instead of stdout', "
                 "dest='_file')\n";
        indent() << "parser.add_argument('--no-preview', action='store_false',"
                 " default=True, help='hide the preview window', "
                 "dest='_preview')\n";

        for (auto a : args) {
            PyUtils::generate_opt(indent(), a);
        }

        indent() << "args = parser.parse_args()\n";
        indent() << "print(args)\n"; /* TODO */
    }


    void PyGtkBackend::generate(ostream &os,
                                const vector<symbol::Argument *> &args,
                                ast::Statements *body)
    {
        if (body) {
            PyHeader h(os);
            h.generate(args);

            PyGuiWriter g(os);
            g.generate(args);

            PyBody b(os);
            b.generate(body);

            PyGtkMain m(os);
            m.generate(args);
        }
    }

    void PyGtkBackend::check_cmd(const vector<symbol::Argument *> &args)
    {
        vector<string> reserved = { "-h", "-o", "--help", "--no-preview" };
        PyUtils::check_cmd(args, reserved);
    }
}
