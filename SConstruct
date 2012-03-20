# vim: set filetype=python:
import os
import subprocess
import platform
import re
import sys
from os.path import join, dirname, abspath
from types import DictType, StringTypes
from glob import glob


AddOption('--prefix',
    dest='prefix',
    nargs=1,
    type='string',
    action='store',
    metavar='DIR',
    default='/usr/local/',
    help='installation prefix'
)

env = Environment(
    LIBS=['re2', 'pthread', 'dl'],
    LIBPATH=['./'],
    CXXFLAGS=['-std=c++0x'],
    CCFLAGS=['-Wall', '-Wno-sign-compare', '-Ivendor/boost_1_49_0/', '-I./vendor/re2/', '-fstack-protector', '-march=native', '-g'],
    PREFIX=GetOption('prefix')
)
print 'PREFIX: ' + env['PREFIX']
re2_env = Environment(
    CCFLAGS=['-pthread', '-Wno-sign-compare', '-O2', '-I./vendor/re2/'],
    LIBS=['pthread'],
)

if os.uname()[0]=='Darwin':
    re2_env.Replace(CXX='clang++', CC='clang')
    env.Replace(CXX='clang++', CC='clang')
    # env.Append(CXXFLAGS=['-Werror'])
    env.Append(
        CCFLAGS=['-Wno-unused-function', '-DBOOST_NO_CHAR16_T', '-DBOOST_NO_CHAR32_T'],
        CXXFLAGS=['-Wno-unneeded-internal-declaration'],
    )
else:
    re2_env.Replace(CXX='g++')
    env.Replace(CXX='g++')
    env.Append(
        LINKFLAGS=['-Wl,-E', '-Wl,-Bsymbolic', '-rdynamic'],
    )

if ARGUMENTS.get('glog', 0):
    env.Append(LIBS=['glog'], CCFLAGS=['-DENABLE_GLOG'])

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
    "src/" + x for x in Split('''
        parser.cc value.cc compiler.cc
        node.cc op.cc vm.cc util.cc
        symbol_table.cc package_map.cc frame.cc package.cc operator.cc
        builtin.cc
        object.cc

        ops.gen.cc token.gen.cc lexer.gen.cc vm.gen.cc nodes.gen.cc symbols.gen.cc

        inspector.cc peek.cc disasm.cc

        value/code.cc value/hash.cc value/str.cc value/array.cc value/regexp.cc value/range.cc
        value/object.cc value/int.cc value/bool.cc value/exception.cc
        value/bytes.cc

        object/str.cc object/array.cc object/dir.cc object/stat.cc object/env.cc object/time.cc object/file.cc object/socket.cc object/internals.cc object/caller.cc object/code.cc object/symbol.cc
        object/dynaloader.cc object/object.cc object/metaclass.cc
        object/bytes.cc object/regexp.cc

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
    env.Command('test', programs, prefix + " " + prove_path + ' --source Tora --source Executable -r tests/ t/tra/*.tra t/tra/*/*.tra --source Perl t')

env.Command('test.valgrind', ['tora'], 'perl misc/valgrind.pl');

if 'bench' in COMMAND_LINE_TARGETS:
    env.Command('bench', [], 'git log --oneline | head -1 && scons ndebug=1 test && ./tora -V && time ./tora benchmark/fib/fib.tra 39')

if 'op' in COMMAND_LINE_TARGETS:
    env.Command('op', [], 'git log --oneline | head -1 && scons && ./tora -V ; sudo opcontrol --reset; sudo opcontrol --start && time ./tora benchmark/fib/fib.tra 39 ; sudo opcontrol --stop')

########
# main programs
env.Command(['src/nodes.gen.h', 'src/nodes.gen.cc'], 'src/nodes.gen.pl', 'perl src/nodes.gen.pl > src/nodes.gen.h');
env.Command(['src/token.gen.cc', 'src/token.gen.h'], ['src/token.gen.pl', 'src/parser.h'], 'perl src/token.gen.pl');
env.Command(['src/lexer.gen.cc'], 'src/lexer.re', 're2c src/lexer.re > src/lexer.gen.cc');
env.Command(['src/vm.gen.cc', 'src/ops.gen.h', 'src/ops.gen.cc'], ['src/vm.gen.pl', 'vm.inc'], 'perl -I misc/Text-MicroTemplate/ src/vm.gen.pl > src/vm.gen.cc');
env.Command(['src/symbols.gen.cc', 'src/symbols.gen.h'], ['src/symbols.gen.pl'], 'perl -I misc/Text-MicroTemplate/ src/symbols.gen.pl');
t = env.Command(['src/parser.h', 'src/parser.cc'], ['lemon', 'src/parser.yy', 'src/lempar.c'], './lemon src/parser.yy');
Clean(t, 'src/parser.out')

libre2 = re2_env.Library('re2', re2files)
# config.h
with open('src/config.h', 'w') as f:
    f.write("#pragma once\n")
    f.write('#define TORA_CCFLAGS "' + ' '.join(env.get('CCFLAGS')) + "\"\n")
    f.write('#define TORA_PREFIX  "' + env.get('PREFIX') + "\"\n")


tora = env.Program('tora', [
    ['src/main.cc'],
    libfiles,
    libre2
])
Default(tora)


# lemon
lemon_env = Environment()
lemon_env.Append(CCFLAGS=['-O2'])
lemon_env.Program('lemon', ['tools/lemon/lemon.c']);

# instalation
env.Install('/usr/local/bin/', 'tora');
env.Alias('install', '/usr/local/bin/')

