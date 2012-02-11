import os
import subprocess
import platform
import re
import sys
from os.path import join, dirname, abspath
from types import DictType, StringTypes

if os.uname()[0]=='Darwin':
    env = Environment(CXX='clang++', CC='clang++')
    env.Append(CXXFLAGS=['-std=c++0x'])
    # env.Append(CXXFLAGS=['-Werror'])
    env.Append(CXXFLAGS=['-Wno-unused-function'])
    env.Append(CXXFLAGS=['-Wno-unneeded-internal-declaration'])
else:
    env = Environment(CXX='g++')
    env.Append(CCFLAGS=['-std=c++0x'])
    env.Append(CXXFLAGS=['-pthread', '-Wno-sign-compare'])
    env.Append(LDFLAGS=['-pthread'])
    env.Append(LINKFLAGS=['-lpthread'])
env.Append(CXXFLAGS=['-I./vendor/re2/'])
env.Append(CCFLAGS=['-g'])
env.Append(CXXFLAGS=['-g'])
env.Append(LDFLAGS=['-g'])
env.Append(CCFLAGS=['-O0'])
env.Append(CCFLAGS=['-Wall'])

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
# env.CXXFile(target='Tora.tab.cc', source='Tora.yy', YACCFLAGS='-dv')
env.Command('ops.gen.h', 'ops.gen.pl', 'perl ops.gen.pl > ops.gen.h');
env.Command(['nodes.gen.h', 'nodes.gen.cc'], 'nodes.gen.pl', 'perl nodes.gen.pl > nodes.gen.h');
env.Command(['lexer.gen.h'], 'lexer.re', 're2c lexer.re > lexer.gen.h');
env.Command('parser.cc', 'parser.yy', 'lemon parser.yy && mv parser.c parser.cc');
env.Program('tora', [
    'main.cc parser.cc value.cc compiler.cc vm.cc array.cc nodes.gen.cc node.cc op.cc ops.gen.cc regexp.cc'.split(' '),
    re2files
])
