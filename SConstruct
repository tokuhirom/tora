# vim: set filetype=python:
import os
import subprocess
import platform
import re
import sys

from os.path import join, dirname, abspath
from types import DictType, StringTypes
from glob import glob

TORA_VERSION_STR='0.0.3'

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
    "tora/" + x for x in Split('''
        parser.cc value.cc compiler.cc
        node.cc op.cc vm.cc util.cc
        symbol_table.cc package_map.cc frame.cc package.cc operator.cc
        builtin.cc
        object.cc pad_list.cc

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

programs = ['bin/tora']
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

env.Command('test.valgrind', ['bin/tora'], 'perl misc/valgrind.pl');

if 'bench' in COMMAND_LINE_TARGETS:
    env.Command('bench', [], 'git log --oneline | head -1 && scons ndebug=1 test && ./bin/tora -V && time ./bin/tora benchmark/fib/fib.tra 39')

if 'op' in COMMAND_LINE_TARGETS:
    env.Command('op', [], 'git log --oneline | head -1 && scons && ./bin/tora -V ; sudo opcontrol --reset; sudo opcontrol --start && time ./bin/tora benchmark/fib/fib.tra 39 ; sudo opcontrol --stop')

########
# main programs
env.Command(['tora/nodes.gen.h', 'tora/nodes.gen.cc'], 'tora/nodes.gen.pl', 'perl tora/nodes.gen.pl > tora/nodes.gen.h');
env.Command(['tora/token.gen.cc', 'tora/token.gen.h'], ['tora/token.gen.pl', 'tora/parser.h'], 'perl tora/token.gen.pl');
env.Command(['tora/lexer.gen.cc'], 'tora/lexer.re', 're2c tora/lexer.re > tora/lexer.gen.cc');
env.Command(['tora/vm.gen.cc', 'tora/ops.gen.h', 'tora/ops.gen.cc'], ['tora/vm.gen.pl', 'vm.inc'], 'perl -I misc/Text-MicroTemplate/ tora/vm.gen.pl > tora/vm.gen.cc');
env.Command(['tora/symbols.gen.cc', 'tora/symbols.gen.h'], ['tora/symbols.gen.pl'], 'perl -I misc/Text-MicroTemplate/ tora/symbols.gen.pl');
t = env.Command(['tora/parser.h', 'tora/parser.cc'], ['tools/lemon/lemon', 'tora/parser.yy', 'tora/lempar.c'], './tools/lemon/lemon tora/parser.yy');
Clean(t, 'tora/parser.out')

libre2 = re2_env.Library('re2', re2files)

TORA_PREFIX   = env.get('PREFIX')
TORA_CC       = env.get('CC')
TORA_CXX      = env.get('CXX')
TORA_CCFLAGS  = ' '.join(env.get('CCFLAGS'))
TORA_CXXFLAGS = ' '.join(env.get('CXXFLAGS'))
# config.h
with open('tora/config.h', 'w') as f:
    f.write("#pragma once\n")
    f.write('#define TORA_CCFLAGS "' + TORA_CCFLAGS + "\"\n")
    f.write('#define TORA_PREFIX  "' + TORA_PREFIX + "\"\n")
    f.write('#define TORA_VERSION_STR  "' + TORA_VERSION_STR + "\"\n")

with open('lib/Config.tra', 'w') as f:
    f.write("my $Config = {\n")
    f.write("  TORA_PREFIX      => '" + TORA_PREFIX      + "',\n")
    f.write("  TORA_CC          => '" + TORA_CC          + "',\n")
    f.write("  TORA_CXX         => '" + TORA_CXX         + "',\n")
    f.write("  TORA_CCFLAGS     => '" + TORA_CCFLAGS     + "',\n")
    f.write("  TORA_CXXFLAGS    => '" + TORA_CXXFLAGS    + "',\n")
    f.write("  TORA_VERSION_STR => '" + TORA_VERSION_STR + "',\n")
    f.write("};\n")
    f.write("sub tora_config() {\n")
    f.write("    $Config;\n")
    f.write("}\n")

tora = env.Program('bin/tora', [
    ['tora/main.cc'],
    libfiles,
    libre2
])
Default(tora)


# lemon
lemon_env = Environment()
lemon_env.Append(CCFLAGS=['-O2'])
lemon_env.Program('tools/lemon/lemon', ['tools/lemon/lemon.c']);

# instalation
installs = []
installs += [env.Install(env['PREFIX']+'/bin/', 'bin/tora')];
installs+=[env.InstallAs(env['PREFIX']+'/lib/tora-'+TORA_VERSION_STR, 'lib/')]
env.Alias('install', [env['PREFIX']+'/bin/', installs])

