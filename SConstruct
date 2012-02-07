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
else:
    env = Environment(CXX='g++')
    env.Append(CCFLAGS=['-std=c++0x'])
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

if 'test' in COMMAND_LINE_TARGETS:
    env.Command('test', 'tora', 'prove -r t')
env.CXXFile(target='Tora.tab.cc', source='Tora.yy', YACCFLAGS='-dv')
env.Command('ops.gen.h', 'ops.gen.pl', 'perl ops.gen.pl > ops.gen.h');
env.Command(['nodes.gen.h', 'nodes.gen.cc'], 'nodes.gen.pl', 'perl nodes.gen.pl > nodes.gen.h');
env.CXXFile(target='lex.yy.cc', source='Tora.ll')
env.Program('tora', ['Tora.tab.cc lex.yy.cc main.cc value.cc compiler.cc vm.cc array.cc nodes.gen.cc node.cc op.cc ops.gen.cc'.split(' ')])
