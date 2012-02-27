import os
import subprocess
import platform
import re
import sys
from os.path import join, dirname, abspath
from types import DictType, StringTypes
from glob import glob

if os.uname()[0]=='Darwin':
    env = Environment(CXX='clang++', CC='clang')
    env.Append(CXXFLAGS=['-std=c++0x'])
    env.Append(CCFLAGS=['-Wall'])
    # env.Append(CXXFLAGS=['-Werror'])
    env.Append(CCFLAGS=['-Wno-unused-function'])
    env.Append(CXXFLAGS=['-Wno-unneeded-internal-declaration'])
else:
    env = Environment(CXX='g++')
    env.Append(CXXFLAGS=['-std=c++0x'])
    env.Append(LDFLAGS=['-pthread'])
    env.Append(LINKFLAGS=['-lpthread'])
    env.Append(CCFLAGS=['-Wall'])
    env.Append(CCFLAGS=['-pthread', '-Wno-sign-compare'])
env.Append(CXXFLAGS=['-I./vendor/re2/'])
env.Append(CCFLAGS=['-g'])
env.Append(CXXFLAGS=['-g'])
env.Append(LDFLAGS=['-g'])

if ARGUMENTS.get('profile', 0):
    env.Append(CXXFLAGS=['-pg', '-Dprofile'])
    env.Append(LINKFLAGS=['-pg'])

if ARGUMENTS.get('ndebug', 1) != '0':
    env.Append(CCFLAGS=['-DNDEBUG'])
    env.Append(CCFLAGS=['-O2'])
else:
    env.Append(CCFLAGS=['-O0'])

# scons debug=1
if ARGUMENTS.get('debug', 0):
    env.Append(CCFLAGS=['-DDEBUG'])

re2files = [
    Glob('vendor/re2/re2/*.cc'),
    'vendor/re2/util/rune.cc',
    'vendor/re2/util/valgrind.cc',
    'vendor/re2/util/stringprintf.cc',
    'vendor/re2/util/strutil.cc',
    'vendor/re2/util/hash.cc',
    'vendor/re2/util/stringpiece.cc',
]
libfiles = [
    "src/" + x for x in Split('''value/range.cc vm.gen.cc value/code.cc value/hash.cc value/str.cc value/array.cc parser.cc value.cc compiler.cc nodes.gen.cc node.cc op.cc ops.gen.cc value/regexp.cc disasm.cc stack.cc
        symbol_table.cc
        value/object.cc
        object/str.cc object/array.cc object/dir.cc object/stat.cc object/env.cc object/json.cc
    ''')
]

########
# tests.
if 'test' in COMMAND_LINE_TARGETS:
    programs = ['tora']
    for src in glob("tests/test_*.cc"):
        programs.append(env.Program(src.rstrip(".cc") + '.t', [
            libfiles,
            re2files,
            src,
        ]))
    prefix = 'PERL5LIB=util/:$PERL5LIB '
    prove_path = 'prove'
    try:
        os.stat('/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.3/bin/prove') # throws exception if not exists
        prove_path = '/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.3/bin/prove'
    except: pass
    env.Command('test', programs, prefix + " " + prove_path + ' --source Tora --source Executable -r tests/ t/tra/*.tra --source Perl t')

########
# main programs
env.Command(['src/nodes.gen.h', 'src/nodes.gen.cc'], 'src/nodes.gen.pl', 'perl src/nodes.gen.pl > src/nodes.gen.h');
env.Command(['src/token.gen.h'], ['src/token.gen.pl', 'src/parser.h'], 'perl src/token.gen.pl');
env.Command(['src/lexer.gen.h'], 'src/lexer.re', 're2c src/lexer.re > src/lexer.gen.h');
env.Command(['src/vm.gen.cc', 'src/ops.gen.h', 'src/ops.gen.cc'], ['src/vm.gen.pl', 'vm.inc'], 'perl src/vm.gen.pl > src/vm.gen.cc');
t = env.Command(['src/parser.h', 'src/parser.cc'], ['src/parser.yy', 'src/lempar.c'], './lemon src/parser.yy && mv src/parser.c src/parser.cc');
Clean(t, 'parser.out')
env.Program('tora', [
    ['src/main.cc'],
    libfiles,
    re2files
])

env.Program('lemon', ['tools/lemon/lemon.c']);
env.Install('/usr/local/bin/', 'tora');
env.Alias('install', '/usr/local/bin/')

