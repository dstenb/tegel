#include "pygtk_backend.hpp"

namespace pygtk_backend
{
    /** Type visitor that generates a comma delimited list of gobject types
     *
     */
    class PyGtkGobjectList : public TypeVisitor
    {
        public:
            PyGtkGobjectList(ostream &os)
                : os_(os) {}

            virtual void visit(const RecordType *p) {
                auto it = p->begin();
                while (it != p->end()) {
                    (*it).type->accept(*this);
                    if (++it != p->end())
                        os_ << ", ";
                }
            }

            virtual void visit(const BoolType *) {
                os_ << "gobject.TYPE_BOOLEAN";
            }

            virtual void visit(const IntType *) {
                os_ << "gobject.TYPE_INT";
            }

            virtual void visit(const StringType *) {
                os_ << "gobject.TYPE_STRING";
            }

            virtual void visit(const ListType *l) {
                l->elem()->accept(*this);
            }
        private:
            ostream &os_;
    };

    /** Type visitor that generates a cell renderer identifier from a
     * primitive type
     *
     */
    class PyGtkRendererType : public TypeVisitor
    {
        public:
            PyGtkRendererType(ostream &os)
                : os_(os) {}

            virtual void visit(const BoolType *) {
                os_ << "toggle";
            }
            virtual void visit(const IntType *) {
                os_ << "spin";
            }
            virtual void visit(const StringType *) {
                os_ << "text";
            }
            virtual void visit(const ListType *) { }
            virtual void visit(const RecordType *) { }
        private:
            ostream &os_;
    };

    /** Type visitor that generates a comma delimited list of column
     * information dictionaries
     *
     */
    class PyGtkColumn : public TypeVisitor
    {
        public:
            PyGtkColumn(ostream &os)
                : os_(os), type_(os) {}

            virtual void visit(const RecordType *p) {
                auto it = p->begin();
                while (it != p->end()) {
                    os_ << "{'type': '";
                    (*it).type->accept(type_);
                    os_ << "', 'label': '" << (*it).name << "'}";
                    if (++it != p->end())
                        os_ << ", ";
                }
            }

            virtual void visit(const BoolType *) {
                os_ << "{'type': 'toggle', 'label': ''}";
            }

            virtual void visit(const IntType *) {
                os_ << "{'type': 'spin', 'label': ''}";
            }

            virtual void visit(const StringType *) {
                os_ << "{'type': 'text', 'label': ''}";
            }

            virtual void visit(const ListType *l) { }
        private:
            ostream &os_;
            PyGtkRendererType type_;
    };

    /** Type visitor that generates a default value from a type
     *
     */
    class PyDefaultDefault : public TypeVisitor
    {
        public:
            PyDefaultDefault(ostream &os)
                : os_(os) {}

            virtual void visit(const RecordType *p) {
                os_ << PyUtils::record_name(p) << "(";
                auto it = p->begin();
                while (it != p->end()) {
                    (*it).type->accept(*this);
                    if (++it != p->end())
                        os_ << ", ";
                }
                os_ << ")";
            }

            virtual void visit(const BoolType *) {
                os_ << "False";
            }

            virtual void visit(const IntType *) {
                os_ << "0";
            }

            virtual void visit(const StringType *) {
                os_ << "''";
            }

            virtual void visit(const ListType *l) { }
        private:
            ostream &os_;
    };

    /** PyGtkHeader is used to generate the additional header information used
     * for pygtk related things
     *
     */
    class PyGtkHeader : public PyWriter
    {
        public:
            PyGtkHeader(ostream &os)
                : PyWriter(os, 0) {}

            void generate(const vector<symbol::Argument *> &args) {
                indent() << "import gobject\n";
                indent() << "import pygtk\n";
                indent() << "pygtk.require('2.0')\n";
                indent() << "import gtk\n";
                indent() << "import cStringIO\n\n";

                indent() << "list_decl = {\n";
                indent_inc();
                for (symbol::Argument *a : args) {
                    auto l = a->get_type()->list();
                    if (l != nullptr) {
                        indent() << "'" << a->get_name() << "': {\n";
                        indent_inc();
                        indent() << "'types': (";
                        PyGtkGobjectList g(unindent());
                        a->get_type()->accept(g);
                        unindent() << ", ),\n";

                        auto r = l->elem()->record();
                        if (r != nullptr) {
                            indent() << "'columns': [";
                            PyGtkColumn c(unindent());
                            r->accept(c);
                            unindent() << "], \n";
                            indent() << "'header': True,\n";
                            indent() << "'record': " <<
                                     PyUtils::record_name(r) << ",\n";
                        } else {
                            indent() << "'columns': [";
                            PyGtkColumn c(unindent());
                            l->elem()->accept(c);
                            unindent() << "], \n";
                            indent() << "'header': False,\n";
                            indent() << "'record': None,\n";
                        }

                        /* Generate the default value for an item in the list
                         */
                        indent() << "'default': ";
                        PyDefaultDefault d(unindent());
                        l->elem()->accept(d);
                        unindent() << "\n";

                        indent_dec();
                        indent() << "},\n";
                    }
                }
                indent_dec();
                indent() << "}\n\n";

                indent() << "def create_default_item(arg_name):\n";
                indent_inc();
                indent() << "d = list_decl[arg_name]['default']\n";
                indent() << "if list_decl[arg_name]['record']:\n";
                indent() << "    return list(d), d\n";
                indent() << "else:\n";
                indent() << "    return [ d ], d\n";
                indent_dec();
            }
    };


    /** Generates the GUI class
     *
     */
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
        gen_create_methods();
    }

    /** Generates GUI.__init__()
     *
     */
    void PyGuiWriter::gen_init(const vector<symbol::Argument *> &args)
    {
        indent() << "def __init__(self, args):\n";
        indent_inc();
        indent() << "self.args = args\n";
        indent() << "self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)\n";
        indent() << "self.window.connect('delete_event', self.delete)\n";
        indent() << "self.window.connect('destroy', self.destroy)\n";
        indent() << "self.window.set_geometry_hints(min_width=800, min_height=600)\n\n";
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

    /** Generate the argument/preview code
     *
     */
    void PyGuiWriter::gen_bottom(const vector<symbol::Argument *> &args)
    {
        indent() << "self.bottom = gtk.HBox()\n";
        indent() << "self.a_scrolled = gtk.ScrolledWindow()\n";
        indent() << "self.a_scrolled.set_policy(gtk.POLICY_AUTOMATIC, "
                 "gtk.POLICY_AUTOMATIC)\n";
        indent() << "self.argument_box = gtk.VBox()\n";
        indent() << "self.a_scrolled.add_with_viewport(self.argument_box)\n";
        indent() << "self.argument_box.show()\n";

        indent() << "self.p_scrolled = gtk.ScrolledWindow()\n";
        indent() << "self.p_scrolled.set_policy(gtk.POLICY_AUTOMATIC, "
                 "gtk.POLICY_AUTOMATIC)\n\n";
        indent() << "self.preview = gtk.TextView()\n";
        indent() << "self.preview.set_editable(False)\n";
        indent() << "self.preview.show()\n";
        indent() << "self.p_scrolled.add_with_viewport(self.preview)\n\n";

        indent() << "self.a_scrolled.show()\n";
        indent() << "self.bottom.add(self.a_scrolled)\n";
        indent() << "self.p_scrolled.set_visible(self.args._preview)\n";
        indent() << "self.bottom.add(self.p_scrolled)\n";
        indent() << "self.bottom.show()\n\n";

        indent() << "for o in [\n";
        for (symbol::Argument *a : args) {
            auto p = a->get("info");
            auto id = static_cast<const StringConstantData *>(p->get());
            auto is = Escaper()(id->value());

            if (is.empty())
                is = a->get_name();

            /* Get the default value */
            auto dd = a->get("default")->get();

            const Type *t = dd->type();

            /* TODO */

            if (t == TypeFactory::get("bool")) {
                indent() << "self.create_bool(\"" << is << "\", \""
                         << a->get_name() << "\"),\n";
            } else if (t == TypeFactory::get("int")) {
                indent() << "self.create_int(\"" << is << "\", \""
                         << a->get_name() << "\"),\n";
            } else if (t == TypeFactory::get("string")) {
                indent() << "self.create_string(\"" << is << "\", \""
                         << a->get_name() << "\"),\n";
            } else if (t->list()) {
                indent() << "self.create_list(\"" << is << "\", \""
                         << a->get_name() << "\"),\n";
            } else if (t->record()) {
                indent() << "self.create_record(\"" << is << "\", \""
                         << a->get_name() << "\"),\n";
            }
        }

        indent() << "]:\n";
        indent() << "    self.argument_box.pack_start(o, expand=False)\n";
        indent() << "    o.show()\n\n";

        /* TODO: Generate arguments */
    }

    /** Generates the menubar code
     *
     */
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

    /** Generates GUI.update()
     *
     */
    void PyGuiWriter::gen_update()
    {
        indent() << "def update(self):\n";
        indent() << "    out = cStringIO.StringIO()\n";
        indent() << "    generate(self.args, out)\n";
        indent() << "    self.preview.get_buffer().set_text(out.getvalue())\n";
        indent() << "    out.close()\n";
    }

    /** Generates GUI.delete()
     *
     */
    void PyGuiWriter::gen_delete()
    {
        indent() << "def delete(self, w, e, d=None):\n";
        indent() << "    return False\n\n";
    }

    /** Generates GUI.destroy()
     *
     */
    void PyGuiWriter::gen_destroy()
    {
        indent() << "def destroy(self, w, d=None):\n";
        indent() << "    gtk.main_quit()\n\n";
    }

    /** Generates GUI.main()
     *
     */
    void PyGuiWriter::gen_main()
    {
        indent() << "def main(self):\n";
        indent() << "    gtk.main()\n\n";
    }

    /** Generates all the save functionality
     *
     */
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
        indent_inc();
        indent() << "try:\n";
        indent() << "    f = open(self.filename, 'w')\n";
        indent() << "    try:\n";
        indent() << "        generate(self.args, f)\n";
        indent() << "    finally:\n";
        indent() << "        f.close()\n";
        indent() << "except IOError:\n";
        indent() << "    dialog = gtk.MessageDialog(self.window, "
                 "gtk.DIALOG_DESTROY_WITH_PARENT, gtk.MESSAGE_ERROR, "
                 "gtk.BUTTONS_CLOSE, 'Couldn\\'t write to %s' % self.filename)\n";
        indent() << "    dialog.run()\n";
        indent() << "    dialog.destroy()\n";
        indent_dec();
    }

    /** Generates the callback functions used by the menu buttons
     *
     */
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

    /** Generates helper functions used by the argument boxes
     *
     */
    void PyGuiWriter::gen_arg_helpers()
    {
        /* Generate create_label(). Used to create gtk.Labels with an uniform
         * appearance */
        indent() << "def create_label(self, string):\n";
        indent_inc();
        indent() << "l = gtk.Label(string)\n";
        indent() << "l.show()\n";
        indent() << "return l\n\n";
        indent_dec();

        /** Generate create_labeled(). Used to create a labeled argument box */
        indent() << "def create_labeled(self, l, *widgets):\n";
        indent() << "    v = gtk.VBox()\n";
        indent() << "    v.pack_start(self.create_label(l), expand=False)\n";
        indent() << "    for w, e in widgets:\n";
        indent() << "        v.pack_start(w, expand=e)\n";
        indent() << "        w.show()\n";
        indent() << "    return v\n\n";

        /* Generate the shared update method for cell data */
        indent() << "def update_list_item(self, name, value, row, i):\n";
        indent() << "    new_item = None\n";
        indent() << "    if list_decl[name]['record']:\n";
        indent() << "        rl = list(getattr(self.args, name)[row])\n";
        indent() << "        rl[i] = value\n";
        indent() << "        new_item = list_decl[name]['record'](*rl)\n";
        indent() << "    else:\n";
        indent() << "        new_item = value\n";
        indent() << "    getattr(self.args, name)[row] = new_item\n";
        indent() << "    self.update()\n\n";
    }

    /** Generates all the callback functions for the argument boxes
     *
     */
    void PyGuiWriter::gen_arg_callbacks()
    {
        /* Generate the callback function for the "add" button */
        indent() << "def add_row(self, w, view, name):\n";
        indent() << "    sv, v = create_default_item(name)\n";
        indent() << "    view.get_model().append(sv)\n";
        indent() << "    getattr(self.args, name).append(v)\n";
        indent() << "    self.update()\n\n";

        /* Generate the callback function for the "remove" button */
        indent() << "def remove_selected(self, w, view, name):\n";
        indent() << "    s, i = view.get_selection().get_selected()\n";
        indent() << "    if i:\n";
        indent() << "        del getattr(self.args, name)[s.get_path(i)[0]]\n";
        indent() << "        s.remove(i)\n";
        indent() << "    self.update()\n\n";

        /* Generate the callback function for a bool argument */
        indent() << "def bool_toggled(self, w, name):\n";
        indent() << "    setattr(self.args, name, w.get_active())\n";
        indent() << "    self.update()\n\n";

        /* Generate the callback function for an int argument */
        indent() << "def int_changed(self, w, name):\n";
        indent() << "    setattr(self.args, name, int(w.get_value()))\n";
        indent() << "    self.update()\n\n";

        /* Generate the callback function for a string argument */
        indent() << "def string_changed(self, w, name):\n";
        indent() << "    setattr(self.args, name, w.get_text())\n";
        indent() << "    self.update()\n\n";

        /* Generate the callback function for a text cell */
        indent() << "def text_cell_edited(self, w, path, text, m, i, name):\n";
        indent() << "    m[path][i] = text\n";
        indent() << "    self.update_list_item(name, text, int(path), i)\n";

        /* Generate the callback function for a spin cell */
        indent() << "def spin_cell_edited(self, w, path, v, m, i, name):\n";
        indent() << "    value = int(v)\n";
        indent() << "    m[path][i] = value\n";
        indent() << "    self.update_list_item(name, value, int(path), i)\n";

        /* Generate the callback function for a toggle cell */
        indent() << "def toggle_cell_edited(self, w, path, m, i, name):\n";
        indent() << "    value = not m[path][i]\n";
        indent() << "    m[path][i] = value\n";
        indent() << "    self.update_list_item(name, value, int(path), i)\n";
    }

    /** Generates the methods used to create argument boxes
     *
     */
    void PyGuiWriter::gen_create_methods()
    {
        gen_arg_helpers();
        gen_arg_callbacks();

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

        indent() << "def create_column(self, i, c, store, arg):\n";
        indent_inc();
        indent() << "print('c = ' + repr(c))\n";
        indent() << "if c['type'] == 'text':\n";
        indent_inc();
        indent() << "cell = gtk.CellRendererText()\n";
        indent() << "cell.set_property('editable', True)\n";
        indent() << "cell.connect('edited', self.text_cell_edited, "
                 "store, i, arg)\n";
        indent() << "return gtk.TreeViewColumn(c['label'], cell, text=i)\n";
        indent_dec();
        indent() << "elif c['type'] == 'toggle':\n";
        indent_inc();
        indent() << "cell = gtk.CellRendererToggle()\n";
        indent() << "cell.set_property('activatable', True)\n";
        indent() << "cell.connect('toggled', self.toggle_cell_edited, "
                 "store, i, arg)\n";
        indent() << "col = gtk.TreeViewColumn(c['label'], active=i)\n";
        indent() << "col.pack_start(cell, False)\n";
        indent() << "col.add_attribute(cell, 'active', i)\n";
        indent() << "return col\n";
        indent_dec();
        indent() << "elif c['type'] == 'spin':\n";
        indent_inc();
        indent() << "cell = gtk.CellRendererSpin()\n";
        indent() << "cell.set_property('editable', True)\n";
        indent() << "a = gtk.Adjustment(0, -65535, 65535, 1)\n";
        indent() << "cell.set_property('adjustment', a)\n";
        indent() << "cell.connect('edited', self.spin_cell_edited, "
                 "store, i, arg)\n";
        indent() << "return gtk.TreeViewColumn(c['label'], cell, text=i)\n";
        indent_dec();
        /* TODO */
        indent_inc();
        indent_dec();
        indent_dec();
        unindent() << "\n";

        indent() << "def create_list(self, label, arg_name):\n";
        indent_inc();
        indent() << "store = gtk.ListStore(*list_decl[arg_name]['types'])\n";
        indent() << "if list_decl[arg_name]['record']:\n";
        indent() << "    for v in getattr(self.args, arg_name):\n";
        indent() << "        store.append(list(v))\n";
        indent() << "else:\n";
        indent() << "    for v in getattr(self.args, arg_name):\n";
        indent() << "        store.append([v])\n";
        indent() << "view = gtk.TreeView(store)\n";
        indent() << "for i, c in enumerate(list_decl[arg_name]['columns']):\n";
        indent() << "    view.append_column(self.create_column(i, c, "
                 "store, arg_name))\n";
        indent() << "b = gtk.HButtonBox()\n";
        indent() << "b.set_layout(gtk.BUTTONBOX_END)\n";
        indent() << "ab = gtk.Button(None, 'gtk-add')\n";
        indent() << "ab.connect('clicked', self.add_row, view, arg_name)\n";
        indent() << "rb = gtk.Button(None, 'gtk-remove')\n";
        indent() << "rb.connect('clicked', self.remove_selected, "
                 "view, arg_name)\n";
        indent() << "for w in [ rb, ab ]:\n";
        indent() << "    w.show()\n";
        indent() << "    b.add(w)\n";
        indent() << "return self.create_labeled(label, "
                 "(view, True), (b, False))\n\n";
        indent_dec();
    }

    void PyGtkMain::generate(const vector<symbol::Argument *> &args)
    {

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
        /* Add "hide preview window" argument */
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
            check_cmd(args);

            PyHeader h(os);
            h.generate(args);

            PyGtkHeader hg(os);
            hg.generate(args);

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
