#!/usr/bin/python

import blib
import os.path
import sys

def configure():
  config = blib.LoadConfig()
  if config is None:
    config = blib.Object()
    config.cflags = ['-fPIC']
    config.libs = []
    # Geany.
    geany = blib.PkgConfig('geany')
    geany.CheckMinVersion('0.19')
    config.cflags.extend(geany.CFlags())
    config.libs.extend(geany.Libs())
    config.install_dir = os.path.join(geany.Variable('libdir').strip(), 'geany')
    # GTK.
    gtk = blib.PkgConfig('gtk+-2.0')
    gtk.CheckMinVersion('0.20')
    config.libs.extend(gtk.CFlags())
    config.libs.extend(gtk.Libs())

    blib.SaveConfig(config)
  return config

def plugin():
  config = configure()
  plugin_lib = blib.SharedLib('geany-java-tags')
  cres_compiler = blib.CResourceCompiler()
  c_compiler = blib.CCompiler(config.cflags)
  plugin_lib.AddObj(c_compiler.Obj(cres_compiler.Resource('java_tags_select_dlg.glade')))
  plugin_lib.AddObj(c_compiler.Obj(cres_compiler.Resource('java_tags_prefs_dlg.glade')))
  plugin_lib.AddObj(c_compiler.Obj('utils.c'))
  plugin_lib.AddObj(c_compiler.Obj('text.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-tags-store.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-tags-prefs.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-tags-prefs-edit.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-tags-parse.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-tags-select.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-parser-common.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-parser-whitespace.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-parser-import.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-parser-package.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-parser-type.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-parser.c'))
  plugin_lib.AddObj(c_compiler.Obj('java-imports.c'))
  plugin_lib.AddObj(c_compiler.Obj('sci-text.c'))
  plugin_lib.AddObj(c_compiler.Obj('sci-utils.c'))
  plugin_lib.AddObj(c_compiler.Obj('plugin.c'))
  plugin_lib.libs.extend(config.libs)
  return plugin_lib

def prefs_dlg():
  config = configure()
  select_bin = blib.Executable('prefs-dlg')
  select_bin.libs.extend(config.libs)
  cresource_compiler = blib.CResourceCompiler()
  c_compiler = blib.CCompiler(config.cflags)
  select_bin.AddObj(c_compiler.Obj(cresource_compiler.Resource('java_tags_prefs_dlg.glade')))
  select_bin.AddObj(c_compiler.Obj('java-tags-prefs.c'))
  select_bin.AddObj(c_compiler.Obj('java-tags-prefs-edit.c'))
  select_bin.AddObj(c_compiler.Obj('java-tags-prefs-main.c'))
  return select_bin

def select_dlg():
  config = configure()
  select_bin = blib.Executable('select-dlg')
  select_bin.libs.extend(config.libs)
  cresource_compiler = blib.CResourceCompiler()
  c_compiler = blib.CCompiler(config.cflags)
  select_bin.AddObj(c_compiler.Obj(cresource_compiler.Resource('java_tags_select_dlg.glade')))
  select_bin.AddObj(c_compiler.Obj('java-tags-store.c'))
  select_bin.AddObj(c_compiler.Obj('java-tags-parse.c'))
  select_bin.AddObj(c_compiler.Obj('java-tags-select.c'))
  select_bin.AddObj(c_compiler.Obj('java-tags-select-main.c'))
  return select_bin

def utils_test():
  config = configure()
  test_bin = blib.Executable('utils-test')
  test_bin.libs.extend(config.libs)
  c_compiler = blib.CCompiler(config.cflags)
  test_bin.AddObj(c_compiler.Obj('utils.c'))
  test_bin.AddObj(c_compiler.Obj('utils-test.c'))
  return test_bin

def parser_test():
  config = configure()
  test_bin = blib.Executable('parser_test')
  test_bin.libs.extend(config.libs)
  c_compiler = blib.CCompiler(config.cflags)
  test_bin.AddObj(c_compiler.Obj('java-tags-store.c'))
  test_bin.AddObj(c_compiler.Obj('java-tags-parse.c'))
  test_bin.AddObj(c_compiler.Obj('java-tags-parse-test.c'))
  return test_bin

def string_text_test():
  config = configure()
  test_bin = blib.Executable('string_text_test')
  test_bin.libs.extend(config.libs)
  c_compiler = blib.CCompiler(config.cflags)
  test_bin.AddObj(c_compiler.Obj('text.c'))
  test_bin.AddObj(c_compiler.Obj('utils.c'))
  test_bin.AddObj(c_compiler.Obj('string-text.c'))
  test_bin.AddObj(c_compiler.Obj('test.c'))
  test_bin.AddObj(c_compiler.Obj('string-text-test.c'))
  return test_bin

def java_parser_test():
  config = configure()
  test_bin = blib.Executable('java_parser_test')
  test_bin.libs.extend(config.libs)
  c_compiler = blib.CCompiler(config.cflags)
  test_bin.AddObj(c_compiler.Obj('utils.c'))
  test_bin.AddObj(c_compiler.Obj('text.c'))
  test_bin.AddObj(c_compiler.Obj('string-text.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-common.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-whitespace.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-import.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-package.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-type.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser.c'))
  test_bin.AddObj(c_compiler.Obj('test.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-test.c'))
  return test_bin

def java_imports_test():
  config = configure()
  test_bin = blib.Executable('java_imports_test')
  test_bin.libs.extend(config.libs)
  c_compiler = blib.CCompiler(config.cflags)
  test_bin.AddObj(c_compiler.Obj('utils.c'))
  test_bin.AddObj(c_compiler.Obj('text.c'))
  test_bin.AddObj(c_compiler.Obj('string-text.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-common.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-whitespace.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-import.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-package.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser-type.c'))
  test_bin.AddObj(c_compiler.Obj('java-parser.c'))
  test_bin.AddObj(c_compiler.Obj('java-imports.c'))
  test_bin.AddObj(c_compiler.Obj('test.c'))
  test_bin.AddObj(c_compiler.Obj('java-imports-test.c'))
  return test_bin

def install():
  config = configure()
  return blib.Copier(plugin(), config.install_dir)

if __name__ == '__main__':
  cmd = sys.argv[1]
  locals()[cmd]().Build()
