#! /usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import argparse
import errno
import os
import sys
import textwrap
from collections import namedtuple

def parse_bool(s):
    return True if s.lower() == "y" else False

def mkdir_chdir(path):
    try:
        os.mkdir(path)
    except OSError as e:
        if e.errno != errno.EEXIST:
            print('Can\'t create %s: %s' % (path, e.strerror))
            sys.exit(1)
    try:
        os.chdir(path)
    except OSError as e:
        print('%s: %s' % (e.strerror, path))

def _input(s = ''):
    if sys.version_info >= (3, 0):
        return input(s)
    else:
        return raw_input(s)

def open_file(path, ask):
    try:
        fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        return os.fdopen(fd, 'w')
    except OSError as e:
        if e.errno == errno.EEXIST:
            while ask:
                c = _input('%s already exists, overwrite? [Y/n]: ' % path)
                if (c in [ 'y', 'Y', '' ]):
                    ask = False
                elif (c in [ 'n', 'N' ]):
                    return None
            return open(path, 'w')
        else:
            raise

def write(f, s):
    if sys.version < '3':
        f.write(s.encode('utf-8'))
    else:
        f.buffer.write(s.encode('utf-8'))

class Loop:
    def __init__(self, list):
        self.list = list
        self.length = len(list)
        self.index = 0
        self.first = True
        self.last = (self.length == 1)

    def tuple(self):
        return tuple_loop(self.index, self.first, self.last, self.length)

    def update(self):
        self.index = self.index + 1
        self.first = False
        self.last = (self.index == self.length - 1)


def copy(o):
    if (isinstance(o, Loop)):
        return o.tuple()
    return o

tuple_loop = namedtuple("tuple_loop", ["index", "first", "last", "length"])

def parse_tuple_loop(s):
    rs = "int:[y/n]:[y/n]:int"
    l = s.split(':')

    if len(l) != 4:
        raise argparse.ArgumentTypeError("Expected a string of type " + rs)
    try:
        return tuple_loop(int(l[0]), parse_bool(l[1]), parse_bool(l[2]), int(l[3]))
    except:
        raise argparse.ArgumentTypeError("Expected a string of type " + rs)

def generate(_args, _file, _body=""):
    if _body == "":
        try:
            f = open_file((_args["name"] + "_backend.hpp"), True)
            __args = {"hpp_file": True, "ast_visitor": _args["ast_visitor"], "name": _args["name"], }
            if f:
                try:
                    generate(__args, f, "backend-file.tgl")
                finally:
                    f.close()
        except IOError as e:
            print('Can\'t create %s: %s' % ((_args["name"] + "_backend.hpp"), e.strerror))
            pass
        try:
            f = open_file((_args["name"] + "_backend.cpp"), True)
            __args = {"hpp_file": False, "ast_visitor": _args["ast_visitor"], "name": _args["name"], }
            if f:
                try:
                    generate(__args, f, "backend-file.tgl")
                finally:
                    f.close()
        except IOError as e:
            print('Can\'t create %s: %s' % ((_args["name"] + "_backend.cpp"), e.strerror))
            pass
    elif _body == "backend-file.tgl":
        a = copy(["Statements", "TernaryIf", "And", "Or", "Not", "BoolEquals", "LessThan", "LessThanOrEqual", "GreaterThan", "GreaterThanOrEqual", "Equals", "Plus", "Minus", "Times", "StringLessThan", "StringLessThanOrEqual", "StringGreaterThan", "StringGreaterThanOrEqual", "StringEquals", "StringRepeat", "StringConcat", "ListConcat", "Constant", "MethodCall", "SymbolRef", "FieldRef", "List", "Record", "FunctionCall", "ExpressionList", "LambdaExpression", "FuncArg", "FuncArgExpression", "FuncArgLambda", "FuncArgList", "Conditional", "Scope", "ForEach", "ForEachEnum", "If", "Elif", "Else", "Text", "InlinedExpression", "VariableStatement", "VariableList", "VariableDeclaration", "VariableAssignment", "Create"])
        write(_file, "")
        if _args["hpp_file"]:
            write(_file, "#ifndef __")
            write(_file, _args["name"].upper())
            write(_file, "_BACKEND_H__\n")
            write(_file, "#define __")
            write(_file, _args["name"].upper())
            write(_file, "_BACKEND_H__\n")
            write(_file, "\n")
            write(_file, "#include <sstream>\n")
            write(_file, "#include <string>\n")
            write(_file, "\n")
            write(_file, "using namespace std;\n")
            write(_file, "\n")
            write(_file, "#include \"ast.hpp\"\n")
            write(_file, "#include \"backend.hpp\"\n")
            write(_file, "#include \"common.hpp\"\n")
            write(_file, "#include \"data.hpp\"\n")
            write(_file, "#include \"type.hpp\"\n")
            write(_file, "\n")
            write(_file, "namespace ")
            write(_file, _args["name"].lower())
            write(_file, "_backend\n")
            write(_file, "{\n")
            if _args["ast_visitor"]:
                write(_file, "    class ")
                write(_file, _args["name"].title())
                write(_file, "Body : public ast::AST_Visitor\n")
                write(_file, "    {\n")
                write(_file, "        public:\n")
                b = Loop(sorted(a, reverse=not True))
                for c in b.list:
                    write(_file, "            virtual void visit(ast::")
                    write(_file, c)
                    write(_file, " *);\n")
                    b.update()
                write(_file, "    };\n")
            write(_file, "\n")
            write(_file, "    class ")
            write(_file, _args["name"].title())
            write(_file, "Backend : public Backend\n")
            write(_file, "    {\n")
            write(_file, "        public:\n")
            write(_file, "            void generate(ostream &, const vector<symbol::Argument *> &,\n")
            write(_file, "                          ast::Statements *);\n")
            write(_file, "    };\n")
            write(_file, "\n")
            write(_file, "    class ")
            write(_file, _args["name"].title())
            write(_file, "TgpBackend : public TgpBackend\n")
            write(_file, "    {\n")
            write(_file, "        public:\n")
            write(_file, "            void generate(ostream &, ParseData *, map<string, ParseData *> &);\n")
            write(_file, "    };\n")
            write(_file, "}\n")
            write(_file, "\n")
            write(_file, "#endif\n")
        else:
            write(_file, "#include \"")
            write(_file, _args["name"].lower())
            write(_file, "_backend.hpp\"\n")
            write(_file, "\n")
            if _args["ast_visitor"]:
                write(_file, "/*\n")
                write(_file, " *\n")
                write(_file, " */\n")
                write(_file, "namespace ")
                write(_file, _args["name"].lower())
                write(_file, "_backend\n")
                write(_file, "{\n")
                d = Loop(a)
                for e in d.list:
                    write(_file, "    void ")
                    write(_file, _args["name"].title())
                    write(_file, "Body::visit(ast::")
                    write(_file, e)
                    write(_file, " *)\n")
                    write(_file, "    {\n")
                    write(_file, "\n")
                    write(_file, "    }\n")
                    if not d.last:
                        write(_file, "\n")
                    d.update()
                write(_file, "}\n")
            write(_file, "\n")
            write(_file, "namespace ")
            write(_file, _args["name"].lower())
            write(_file, "_backend\n")
            write(_file, "{\n")
            write(_file, "    void ")
            write(_file, _args["name"].title())
            write(_file, "Backend::generate(ostream &os,\n")
            write(_file, "                             const vector<symbol::Argument *> &args,\n")
            write(_file, "                             ast::Statements *body)\n")
            write(_file, "    {\n")
            write(_file, "\n")
            write(_file, "    }\n")
            write(_file, "\n")
            write(_file, "    void ")
            write(_file, _args["name"].title())
            write(_file, "TgpBackend::generate(ostream &os, ParseData *tgp_data,\n")
            write(_file, "                                map<string, ParseData *> &tgl_data)\n")
            write(_file, "    {\n")
            write(_file, "\n")
            write(_file, "    }\n")
            write(_file, "}\n")

def main(argv=None):
    if argv is None:
       argv = sys.argv

    cmd = argv[0]

    parser = argparse.ArgumentParser(description="Generated by TeGeL.")
    parser.add_argument('-o', type=argparse.FileType('w'), default=sys.stdout, help='output to file instead of stdout', dest='_file')
    parser.add_argument('--dir', type=str, default='.', help='select output directory (will create the directory if non-existant)', dest='_dir')
    parser.add_argument("-n", type=str, default="", help="Backend name", dest="name")
    parser.add_argument("-a", type=parse_bool, default=False, help="Generate a AST visitor?", dest="ast_visitor")
    args = parser.parse_args()

    mkdir_chdir(args._dir)

    if sys.version < '3':
        for a in vars(args):
            v = getattr(args, a)
            if not a.startswith('_'):
                if (type(v) == str):
                    setattr(args, a, v.decode(sys.stdin.encoding))
                elif not type(v) in [ bool, int ]:
                    l = [ (v[i].decode('utf-8') if (type(v[i]) == str)
                        else v[i]) for i in range(len(v)) ]
                    setattr(args, a, v.__new__(type(v), *l))

    generate(vars(args), args._file)

if __name__ == "__main__":
    main()
