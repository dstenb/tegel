#! /usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import sys
import textwrap
from collections import namedtuple

def parse_bool(s):
    return True if s.lower() == "y" else False

tuple_author = namedtuple("tuple_author", ["name", "mail"])

def parse_tuple_author(s):
    rs = "string:string"
    l = s.split(':')

    if len(l) != 2:
        raise argparse.ArgumentTypeError("Expected a string of type " + rs)
    try:
        return tuple_author(l[0], l[1])
    except:
        raise argparse.ArgumentTypeError("Expected a string of type " + rs)
tuple_function = namedtuple("tuple_function", ["returns", "name", "parameters"])

def parse_tuple_function(s):
    rs = "string:string:string"
    l = s.split(':')

    if len(l) != 3:
        raise argparse.ArgumentTypeError("Expected a string of type " + rs)
    try:
        return tuple_function(l[0], l[1], l[2])
    except:
        raise argparse.ArgumentTypeError("Expected a string of type " + rs)
import gobject
import pygtk
pygtk.require('2.0')
import gtk
import pango
import cStringIO

list_decl = {
    'functions': {
        'types': (gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_STRING, ),
        'columns': [{'type': 'text', 'label': 'returns'}, {'type': 'text', 'label': 'name'}, {'type': 'text', 'label': 'parameters'}], 
        'header': True,
        'record': tuple_function,
        'default': tuple_function('', '', '')
    },
    'includes': {
        'types': (gobject.TYPE_STRING, ),
        'columns': [{'type': 'text', 'label': ''}], 
        'header': False,
        'record': None,
        'default': ''
    },
}

def create_default_item(arg_name):
    d = list_decl[arg_name]['default']
    if list_decl[arg_name]['record']:
        return list(d), d
    else:
        return [ d ], d
class GUI:
    def __init__(self, args):
        self.args = args
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect('delete_event', self.delete)
        self.window.connect('destroy', self.destroy)
        self.window.set_geometry_hints(min_width=800, min_height=600)

        self.window.show()

        self.filename = None

        self.top = gtk.MenuBar()
        accelg = gtk.AccelGroup()
        self.window.add_accel_group(accelg)
        filemenu = gtk.Menu()
        file = gtk.MenuItem('File')
        file.set_submenu(filemenu)
        save = gtk.ImageMenuItem(gtk.STOCK_SAVE, accelg)
        save.connect('activate', self.save_callback)
        k, m = gtk.accelerator_parse('<Control>S')
        save.add_accelerator('activate', accelg, k, m, gtk.ACCEL_VISIBLE)
        save_as = gtk.ImageMenuItem(gtk.STOCK_SAVE_AS, accelg)
        save_as.connect('activate', self.save_as_callback)
        k, m = gtk.accelerator_parse('<Shift><Control>S')
        save_as.add_accelerator('activate', accelg, k, m, gtk.ACCEL_VISIBLE)
        exit = gtk.ImageMenuItem(gtk.STOCK_QUIT, accelg)
        exit.connect('activate', self.destroy)
        k, m = gtk.accelerator_parse('<Control>Q')
        exit.add_accelerator('activate', accelg, k, m, gtk.ACCEL_VISIBLE)
        for o in [ save, save_as, gtk.SeparatorMenuItem(), exit ]:
            filemenu.append(o)
        viewmenu = gtk.Menu()
        view = gtk.MenuItem('View')
        view.set_submenu(viewmenu)
        preview = gtk.CheckMenuItem('Preview')
        preview.set_active(self.args['_preview'])
        preview.connect('activate', self.toggle_preview)
        viewmenu.append(preview)
        k, m = gtk.accelerator_parse('<Control>P')
        preview.add_accelerator('activate', accelg, k, m, gtk.ACCEL_VISIBLE)
        self.top.append(file)
        self.top.append(view)
        self.top.show_all()
        self.bottom = gtk.HBox()
        self.a_scrolled = gtk.ScrolledWindow()
        self.a_scrolled.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.argument_box = gtk.VBox()
        self.a_scrolled.add_with_viewport(self.argument_box)
        self.argument_box.show()
        self.p_scrolled = gtk.ScrolledWindow()
        self.p_scrolled.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)

        self.preview = gtk.TextView()
        self.preview.set_editable(False)
        self.preview.show()
        font = pango.FontDescription('monospace')
        self.preview.modify_font(font)
        self.p_scrolled.add_with_viewport(self.preview)

        self.a_scrolled.show()
        self.bottom.add(self.a_scrolled)
        self.p_scrolled.set_visible(self.args['_preview'])
        self.bottom.add(self.p_scrolled)
        self.bottom.show()

        for o in [
        self.create_record('author', 'Author information', [
            {'label': 'name', 'type': 'text', 'name': 'name'}, 
            {'label': 'mail', 'type': 'text', 'name': 'mail'}
        ]),
        self.create_string("Program version", "version"),
        self.create_string("File description", "description"),
        self.create_list("List of functions", "functions"),
        self.create_list("Includes", "includes"),
        self.create_bool("Generate main function?", "main"),
        self.create_bool("Include main() arguments?", "main_a"),
        ]:
            self.argument_box.pack_start(o, expand=False)
            o.show()

        self.vbox = gtk.VBox()
        self.vbox.pack_start(self.top, expand=False)
        self.vbox.add(self.bottom)
        self.vbox.show()
        self.window.add(self.vbox)

        if self.args['_preview']:
            self.update()

    def update(self):
        out = cStringIO.StringIO()
        generate(self.args, out)
        self.preview.get_buffer().set_text(out.getvalue())
        out.close()
    def delete(self, w, e, d=None):
        return False

    def destroy(self, w, d=None):
        gtk.main_quit()

    def save_dialog(self):
        dialog = gtk.FileChooserDialog('Save', None, gtk.FILE_CHOOSER_ACTION_SAVE, (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_SAVE, gtk.RESPONSE_ACCEPT))
        dialog.set_do_overwrite_confirmation(True)
        dialog.set_modal(True)
        dialog.connect('response', self.save_dialog_response)
        if self.filename:
            dialog.set_filename(self.filename)
        else:
            dialog.set_current_name('Untitled')
        dialog.run()

    def save_dialog_response(self, dialog, response):
        if response == gtk.RESPONSE_ACCEPT:
            self.filename = dialog.get_filename()
            self.write_to_file()
        dialog.destroy()

    def write_to_file(self):
        try:
            f = open(self.filename, 'w')
            try:
                generate(self.args, f)
            finally:
                f.close()
        except IOError:
            dialog = gtk.MessageDialog(self.window, gtk.DIALOG_DESTROY_WITH_PARENT, gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, 'Couldn\'t write to %s' % self.filename)
            dialog.run()
            dialog.destroy()
    def toggle_preview(self, w, data=None):
        self.p_scrolled.set_visible(not self.p_scrolled.get_visible())

    def save_callback(self, w, data=None):
        if self.filename:
            self.write_to_file()
        else:
            self.save_dialog()

    def save_as_callback(self, w, data=None):
        self.save_dialog()

    def main(self):
        gtk.main()

    def create_label(self, string):
        l = gtk.Label(string)
        l.show()
        return l

    def create_labeled(self, l, *widgets):
        v = gtk.VBox()
        v.pack_start(self.create_label(l), expand=False)
        for w, e in widgets:
            v.pack_start(w, expand=e)
            w.show()
        return v

    def create_toggle(self, value, cb, *cb_args):
        b = gtk.CheckButton(None)
        b.set_active(value)
        b.connect('toggled', cb, *cb_args)
        return b

    def create_spin(self, value, cb, *cb_args):
        a = gtk.Adjustment(0, -65535, 65535, 1)
        i = gtk.SpinButton(a)
        i.set_value(value)
        i.connect('value-changed', cb, *cb_args)
        return i

    def create_text(self, value, cb, *cb_args):
        e = gtk.Entry()
        e.set_text(value)
        e.connect('changed', cb, *cb_args)
        return e

    def update_list_item(self, name, value, row, i):
        new_item = None
        if list_decl[name]['record']:
            rl = list(self.args[name][row])
            rl[i] = value
            new_item = list_decl[name]['record'](*rl)
        else:
            new_item = value
        self.args[name][row] = new_item
        self.update()

    def create_text_column(self, l, i, store, name):
        cell = gtk.CellRendererText()
        cell.set_property('editable', True)
        cell.connect('edited', self.text_cell_edited, store, i, name)
        return gtk.TreeViewColumn(l, cell, text=i)
    def create_toggle_column(self, l, i, store, name):
        cell = gtk.CellRendererToggle()
        cell.set_property('activatable', True)
        cell.connect('toggled', self.toggle_cell_edited, store, i, name)
        col = gtk.TreeViewColumn(l, active=i)
        col.pack_start(cell, False)
        col.add_attribute(cell, 'active', i)
        return col

    def create_spin_column(self, l, i, store, name):
        cell = gtk.CellRendererSpin()
        cell.set_property('editable', True)
        a = gtk.Adjustment(0, -65535, 65535, 1)
        cell.set_property('adjustment', a)
        cell.connect('edited', self.spin_cell_edited, store, i, name)
        return gtk.TreeViewColumn(l, cell, text=i)

    def create_column(self, i, c, store, name):
        d = { 'text': self.create_text_column,
              'toggle': self.create_toggle_column, 
              'spin': self.create_spin_column }
        return d[c['type']](c['label'], i, store, name)

    def create_store(self, name):
        store = gtk.ListStore(*list_decl[name]['types'])
        if list_decl[name]['record']:
            for v in self.args[name]:
                store.append(list(v))
        else:
            for v in self.args[name]:
                store.append([v])
        return store

    def create_view(self, name, store):
        view = gtk.TreeView(store)
        for i, c in enumerate(list_decl[name]['columns']):
            view.append_column(self.create_column(i, c, store, name))
        return view

    def create_list_buttons(self, name, view):
        box = gtk.HButtonBox()
        box.set_layout(gtk.BUTTONBOX_END)
        add = gtk.Button(None, 'gtk-add')
        add.connect('clicked', self.add_row, view, name)
        remove = gtk.Button(None, 'gtk-remove')
        remove.connect('clicked', self.remove_selected, view, name)
        for w in [ remove, add ]:
            w.show()
            box.add(w)
        return box

    def register_change(self, name, i, v):
        rl = list(self.args[name])
        rl[i] = v
        self.args[name] = type(self.args[name])(*rl)
        self.update()

    def add_row(self, w, view, name):
        sv, v = create_default_item(name)
        view.get_model().append(sv)
        self.args[name].append(v)
        self.update()

    def remove_selected(self, w, view, name):
        s, i = view.get_selection().get_selected()
        if i:
            del self.args[name][s.get_path(i)[0]]
            s.remove(i)
        self.update()

    def bool_toggled(self, w, name):
        self.args[name] = w.get_active()
        self.update()

    def int_changed(self, w, name):
        self.args[name] = int(w.get_value())
        self.update()

    def string_changed(self, w, name):
        self.args[name] = w.get_text()
        self.update()

    def text_cell_edited(self, w, path, text, m, i, name):
        m[path][i] = text
        self.update_list_item(name, text, int(path), i)
    def spin_cell_edited(self, w, path, v, m, i, name):
        value = int(v)
        m[path][i] = value
        self.update_list_item(name, value, int(path), i)
    def toggle_cell_edited(self, w, path, m, i, name):
        value = not m[path][i]
        m[path][i] = value
        self.update_list_item(name, value, int(path), i)
    def bool_field_changed(self, w, name, i):
        self.register_change(name, i, w.get_active())

    def int_field_changed(self, w, name, i):
        self.register_change(name, i, int(w.get_value()))

    def string_field_changed(self, w, name, i):
        self.register_change(name, i, w.get_text())

    def create_bool(self, label, name):
        b = self.create_toggle(self.args[name], self.bool_toggled, name)
        return self.create_labeled(label, (b, False))

    def create_int(self, label, name):
        i = self.create_spin(self.args[name], self.int_changed, name)
        return self.create_labeled(label, (i, False))

    def create_string(self, label, name):
        e = self.create_text(self.args[name], self.string_changed, name)
        return self.create_labeled(label, (e, False))

    def create_list(self, label, name):
        view = self.create_view(name, self.create_store(name))
        buttons = self.create_list_buttons(name, view)
        return self.create_labeled(label, (view, True), (buttons, False))

    def create_record(self, name, label, fields):
        table = gtk.Table(len(fields), 2)
        d = { 'text': (self.create_text, self.string_field_changed),
              'toggle': (self.create_toggle, self.bool_field_changed),
              'spin': (self.create_spin, self.int_field_changed) }
        for i, f in enumerate(fields):
            t = d[f['type']]
            w = t[0](self.args[name][i], t[1], name, i)
            l = gtk.Label(f['label'])
            table.attach(l, 0, 1, i, i+1, xoptions=0, xpadding=9)
            table.attach(w, 1, 2, i, i+1)
        table.show_all()
        return self.create_labeled(label, (table, True))
def generate(_args, _file):
    _file.write('/**\n')
    _file.write(' *\n')
    if not (_args["author"].name == ""):
        a = _args["author"].mail.replace("@", " at ").replace(".", " dot ")
        _file.write(' * @author ')
        _file.write((_args["author"].name + (((" <" + a) + ">") if not (a == "") else "")))
        _file.write('\n')
    if not (_args["version"] == ""):
        _file.write(' * @version ')
        _file.write(_args["version"])
        _file.write('\n')
    _file.write(' *\n')
    if not (_args["description"] == ""):
        _file.write(' * @section DESCRIPTION\n')
        for b in textwrap.wrap(_args["description"], 50):
            _file.write(' * ')
            _file.write(b)
            _file.write('\n')
    _file.write(' */\n')
    _file.write('\n')
    for c in _args["includes"]:
        _file.write('#include ')
        _file.write(c)
        _file.write('\n')
    _file.write('\n')
    for d in _args["functions"]:
        if not (d.name == ""):
            _file.write('/**\n')
            _file.write(' * TODO\n')
            _file.write(' *\n')
            if not (d.returns == "void"):
                _file.write(' * @return TODO\n')
            _file.write(' */\n')
            e = (d.parameters if not (d.parameters == "") else "void")
            _file.write((((((d.returns + " ") + d.name) + "(") + e) + ")"))
            _file.write('\n')
            _file.write('{\n')
            _file.write('\n')
            _file.write('}\n')
    _file.write('\n')
    if _args["main"]:
        _file.write('int main(')
        _file.write(("int argc, char **argv" if _args["main_a"] else "void"))
        _file.write(')\n')
        _file.write('{\n')
        _file.write('\treturn 0;\n')
        _file.write('}\n')
def main(argv=None):
    if argv is None:
       argv = sys.argv

    cmd = argv[0]

    parser = argparse.ArgumentParser(description="Generated by TeGeL.")
    parser.add_argument('-o', type=argparse.FileType('w'), default=sys.stdout, help='output to file instead of stdout', dest='_file')
    parser.add_argument('--no-preview', action='store_false', default=True, help='hide the preview window', dest='_preview')
    parser.add_argument("--author", type=parse_tuple_author, default=tuple_author("", ""), help="Author information", dest="author")
    parser.add_argument("-v", type=str, default="", help="Program version", dest="version")
    parser.add_argument("-d", type=str, default="", help="File description", dest="description")
    parser.add_argument("-f", nargs="+", type=parse_tuple_function, default=[], help="List of functions", dest="functions")
    parser.add_argument("-i", nargs="+", type=str, default=["<stdlib.h>", "<stdio.h>"], help="Includes", dest="includes")
    parser.add_argument("-m", type=parse_bool, default=True, help="Generate main function?", dest="main")
    parser.add_argument("-a", type=parse_bool, default=True, help="Include main() arguments?", dest="main_a")
    args = parser.parse_args()

    g = GUI(vars(args))
    g.main()
if __name__ == "__main__":
    main()
