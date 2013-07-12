#! /usr/bin/python

import pygtk
pygtk.require('2.0')
import gtk

descriptions = {
    "custom": "Use custom character?",
    "width": "Width",
    "custom_character": "Custom character"
    }

class C:
    custom = True
    width = 10
    custom_character = "X"

args = C()

class GUI:

    def __init__(self, args):
        self.args = args

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect('delete_event', self.delete)
        self.window.connect('destroy', self.destroy)
        self.window.show()
        self.window.set_title("TeGeL PyGTK")

        self.filename = None

        self.vbox = gtk.VBox()
        self.vbox.show()

        self.create_top()
        self.create_bottom()

        self.window.add(self.vbox)
        self.vbox.pack_start(self.top, expand=False)
        self.vbox.add(self.bottom)

    def update(self):
        # This will be replaced with the generate() function
        # TODO: use StringIO.StringIO or cStringIO.StringIO
        s = ""
        c = self.args.custom_character if self.args.custom else "X"

        for i in range(0, self.args.width):
            s += c * (i + 1) + "\n"
        for i in reversed(range(0, self.args.width - 1)):
            s += c * (i + 1) + "\n"
        self.preview.get_buffer().set_text(s)

    # Helper function for a label widget
    def create_label(self, string):
        l = gtk.Label(string)
        l.show()
        return l

    # Create a labeled widget
    def create_labeled(self, label, o):
        v = gtk.VBox()
        v.pack_start(self.create_label(label), expand=False)
        v.pack_start(o, expand=False)
        o.show()
        return v

    # Create a bool argument widget
    def create_bool(self, label, arg_name):
        b = gtk.CheckButton(label)
        b.set_active(getattr(self.args, arg_name))
        b.connect('toggled', self.bool_toggled, arg_name)
        return b

    # Create an int argument widget
    def create_int(self, label, arg_name):
        a = gtk.Adjustment(getattr(self.args, arg_name), -65535, 65535, 1)
        i = gtk.SpinButton(a)
        i.set_value(getattr(self.args, arg_name))
        i.connect('value-changed', self.int_changed, arg_name)
        return self.create_labeled(label, i)

    # Create a string argument widget
    def create_string(self, label, arg_name):
        e = gtk.Entry()
        e.set_text(getattr(self.args, arg_name))
        e.connect('changed', self.string_changed, arg_name)
        return self.create_labeled(label, e)

    # Callback for bool. Save the value and update the preview window
    def bool_toggled(self, w, name):
        setattr(self.args, name, w.get_active())
        print(name + ' = ' + str(getattr(self.args, name)))
        self.update()

    # Callback for int. Save the value and update the preview window
    def int_changed(self, w, name):
        setattr(self.args, name, int(w.get_value()))
        print(name + ' = ' + str(getattr(self.args, name)))
        self.update()

    # Callback for string. Save the value and update the preview window
    def string_changed(self, w, name):
        setattr(self.args, name, w.get_text())
        print(name + ' = ' + getattr(self.args, name))
        self.update()

    # Create the bottom box (argument list and preview window)
    def create_bottom(self):
        self.bottom = gtk.HPaned()

        # Create the argument window
        self.a_scrolled = gtk.ScrolledWindow()
        self.a_scrolled.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)

        self.argument_box = gtk.VBox()
        self.a_scrolled.add_with_viewport(self.argument_box)
        self.argument_box.show()

        def p(s):
            return (descriptions[s], s)
        for o in [
                self.create_bool(*p('custom')),
                self.create_int(*p('width')),
                self.create_string(*p('custom_character')) ]:
            self.argument_box.pack_start(o, expand=False)
            o.show()

        # Create the preview window
        self.p_scrolled = gtk.ScrolledWindow()
        self.p_scrolled.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)

        self.preview = gtk.TextView()
        self.preview.set_editable(False)
        self.preview.show()
        self.p_scrolled.add_with_viewport(self.preview)

        for b in [ self.a_scrolled, self.p_scrolled ]:
            self.bottom.add(b)
            b.show()
        self.bottom.show()

    # Create the top box (buttons)
    def create_top(self):
        self.top = gtk.MenuBar()

        accelg = gtk.AccelGroup()
        self.window.add_accel_group(accelg)

        filemenu = gtk.Menu()
        file = gtk.MenuItem("File")
        file.set_submenu(filemenu)

        save = gtk.ImageMenuItem(gtk.STOCK_SAVE, accelg)
        save.connect('activate', self.save_callback)
        k, m = gtk.accelerator_parse("<Control>S")
        save.add_accelerator("activate", accelg, k, m, gtk.ACCEL_VISIBLE)

        save_as = gtk.ImageMenuItem(gtk.STOCK_SAVE_AS, accelg)
        save_as.connect('activate', self.save_as_callback)
        k, m = gtk.accelerator_parse("<Shift><Control>S")
        save_as.add_accelerator("activate", accelg, k, m, gtk.ACCEL_VISIBLE)

        exit = gtk.ImageMenuItem(gtk.STOCK_QUIT, accelg)
        exit.connect('activate', self.destroy)
        k, m = gtk.accelerator_parse("<Control>Q")
        exit.add_accelerator("activate", accelg, k, m, gtk.ACCEL_VISIBLE)

        for o in [ save, save_as, gtk.SeparatorMenuItem(), exit ]:
            filemenu.append(o)

        viewmenu = gtk.Menu()
        view = gtk.MenuItem("View")
        view.set_submenu(viewmenu)

        preview = gtk.CheckMenuItem("Preview")
        preview.connect("activate", self.toggle_preview)
        viewmenu.append(preview)
        k, m = gtk.accelerator_parse("<Control>P")
        preview.add_accelerator("activate", accelg, k, m, gtk.ACCEL_VISIBLE)

        self.top.append(file)
        self.top.append(view)

        self.top.show_all()

    def delete(self, w, e, data=None):
        return False

    def destroy(self, w, data=None):
        gtk.main_quit()

    def toggle_preview(self, w, data=None):
        self.p_scrolled.set_visible(not self.p_scrolled.get_visible())

    def save_dialog(self):
        dialog = gtk.FileChooserDialog('Save',
                None, gtk.FILE_CHOOSER_ACTION_SAVE,
                (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                    gtk.STOCK_SAVE, gtk.RESPONSE_ACCEPT))
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

    def save_callback(self, w, data=None):
        if self.filename:
            self.write_to_file()
        else:
            self.save_dialog()

    def save_as_callback(self, w, data=None):
        self.save_dialog()

    def write_to_file(self):
        print("Writing to " + self.filename)

    def main(self):
        gtk.main()

if __name__ == '__main__':
    gui = GUI(args)
    gui.main()
