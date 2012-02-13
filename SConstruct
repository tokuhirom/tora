import os
import subprocess
import platform
import re
import sys
from os.path import join, dirname, abspath
from types import DictType, StringTypes

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
    env.Append(CXXFLAGS=['-pthread', '-Wno-sign-compare'])
    env.Append(LDFLAGS=['-pthread'])
    env.Append(LINKFLAGS=['-lpthread'])
    env.Append(CCFLAGS=['-Wall'])
env.Append(CXXFLAGS=['-I./vendor/re2/'])
env.Append(CCFLAGS=['-g'])
env.Append(CXXFLAGS=['-g'])
env.Append(LDFLAGS=['-g'])
env.Append(CCFLAGS=['-O2'])

if ARGUMENTS.get('profile', 0):
    env.Append(CXXFLAGS=['-pg'])
    env.Append(LINKFLAGS=['-pg'])

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

if 'test' in COMMAND_LINE_TARGETS:
    env.Command('test', 'tora', 'prove -r t')
env.Command(['src/nodes.gen.h', 'src/nodes.gen.cc'], 'src/nodes.gen.pl', 'perl src/nodes.gen.pl > src/nodes.gen.h');
env.Command(['src/lexer.gen.h'], 'src/lexer.re', 're2c src/lexer.re > src/lexer.gen.h');
env.Command(['src/vm.gen.cc', 'src/ops.gen.h', 'src/ops.gen.cc'], ['src/vm.gen.pl', 'vm.inc'], 'perl src/vm.gen.pl > src/vm.gen.cc');
t = env.Command(['src/parser.h', 'src/parser.cc'], ['src/parser.yy', 'src/lempar.c'], './lemon src/parser.yy && mv src/parser.c src/parser.cc');
Clean(t, 'parser.out')
env.Program('tora', [
    ["src/" + x for x in 'vm.gen.cc value/code.cc value/hash.cc value/str.cc value/array.cc parser.cc main.cc value.cc compiler.cc nodes.gen.cc node.cc op.cc ops.gen.cc regexp.cc disasm.cc stack.cc'.split(' ')],
    re2files
])
env.Program('lemon', ['tools/lemon/lemon.c']);

