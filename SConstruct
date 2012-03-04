# vim: set filetype=python:
import os
import subprocess
import platform
import re
import sys
from os.path import join, dirname, abspath
from types import DictType, StringTypes
from glob import glob

env = Environment(
    LIBS=['re2', 'pthread'],
    LIBPATH=['./'],
    CXXFLAGS=['-std=c++0x'],
    CCFLAGS=['-Wall', '-Wno-sign-compare', '-I./vendor/re2/', '-static', '-fstack-protector', '-march=native', '-g'],
)
re2_env = Environment(
    CCFLAGS=['-pthread', '-Wno-sign-compare', '-O2', '-I./vendor/re2/'],
    LIBS=['pthread'],
)

if os.uname()[0]=='Darwin':
    re2_env.Replace(CXX='clang++', CC='clang')
    env.Replace(CXX='clang++', CC='clang')
    # env.Append(CXXFLAGS=['-Werror'])
    env.Append(CCFLAGS=['-Wno-unused-function'])
    env.Append(CXXFLAGS=['-Wno-unneeded-internal-declaration'])
else:
    re2_env.Replace(CXX='g++')
    env.Replace(CXX='g++')

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
    env.Append(CCFLAGS=['-g'])

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
        token.gen.cc value.gen.cc
        symbol_table.cc
        value/object.cc
        object/str.cc object/array.cc object/dir.cc object/stat.cc object/env.cc object/json.cc object/time.cc object/file.cc object/socket.cc
        vm.cc util.cc
    ''')
]


########
# tests.

programs = ['tora']
for src in glob("tests/test_*.cc"):
    programs.append(env.Program(src.rstrip(".cc") + '.t', [
        libfiles,
        src,
    ]))

if 'test' in COMMAND_LINE_TARGETS:
    prefix = 'PERL5LIB=util/:$PERL5LIB '
    prove_path = 'prove'
    try:
        os.stat('/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.3/bin/prove') # throws exception if not exists
        prove_path = '/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.3/bin/prove'
    except: pass
    try:
        os.stat('/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.2/bin/prove')
        prove_path = '/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.2/bin/prove'
    except: pass
    env.Command('test', programs, prefix + " " + prove_path + ' --source Tora --source Executable -r tests/ t/tra/*.tra --source Perl t')

if 'bench' in COMMAND_LINE_TARGETS:
    env.Command('bench', [], 'git log --oneline | head -1 && scons ndebug=1 test && ./tora -V && time ./tora benchmark/fib/fib.tra 39')

if 'op' in COMMAND_LINE_TARGETS:
    env.Command('op', [], 'git log --oneline | head -1 && scons && ./tora -V ; sudo opcontrol --reset; sudo opcontrol --start && time ./tora benchmark/fib/fib.tra 39 ; sudo opcontrol --stop')

########
# main programs
env.Command(['src/nodes.gen.h', 'src/nodes.gen.cc'], 'src/nodes.gen.pl', 'perl src/nodes.gen.pl > src/nodes.gen.h');
env.Command(['src/value.gen.cc'], 'src/value.gen.pl', 'perl src/value.gen.pl');
env.Command(['src/token.gen.cc', 'src/token.gen.h'], ['src/token.gen.pl', 'src/parser.h'], 'perl src/token.gen.pl');
env.Command(['src/lexer.gen.h'], 'src/lexer.re', 're2c src/lexer.re > src/lexer.gen.h');
env.Command(['src/vm.gen.cc', 'src/ops.gen.h', 'src/ops.gen.cc'], ['src/vm.gen.pl', 'vm.inc'], 'perl -I misc/Text-MicroTemplate/ src/vm.gen.pl > src/vm.gen.cc');
t = env.Command(['src/parser.h', 'src/parser.cc'], ['lemon', 'src/parser.yy', 'src/lempar.c'], './lemon src/parser.yy && mv src/parser.c src/parser.cc');
Clean(t, 'src/parser.out')

lib = re2_env.Library('re2', re2files)
with open('src/config.h', 'w') as f:
    f.write("#pragma once\n")
    f.write('#define TORA_CCFLAGS "' + ' '.join(env.get('CCFLAGS')) + "\"\n")
tora = env.Program('tora', [
    ['src/main.cc'],
    libfiles,
    lib
])
Default(tora)


# lemon
lemon_env = Environment()
lemon_env.Append(CCFLAGS=['-O2'])
lemon_env.Program('lemon', ['tools/lemon/lemon.c']);

# instalation
env.Install('/usr/local/bin/', 'tora');
env.Alias('install', '/usr/local/bin/')

