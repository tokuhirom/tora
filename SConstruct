# vim: set filetype=python:
import os
import subprocess
import platform
import re
import sys
import json

from os.path import join, dirname, abspath
from types import DictType, StringTypes
from glob import glob

TORA_VERSION_STR='0.0.5'

tools = ['default']
default_prefix = '/usr/local'
if os.name == 'nt':
    tools = ['mingw']
    default_prefix = 'c:/tora'

AddOption('--prefix',
    dest='prefix',
    nargs=1,
    type='string',
    action='store',
    metavar='DIR',
    default=default_prefix,
    help='installation prefix'
)

if os.name == 'nt':
    env = Environment(
        ENV={'PATH': os.environ['PATH']},
        LIBS=['re2', 'pthread', 'ws2_32'],
        LIBPATH=['./'],
        CXXFLAGS=['-std=c++0x'],
        CCFLAGS=['-Wall', '-Wno-sign-compare', '-I' + os.getcwd() + '/vendor/boost_1_49_0/', '-I' + os.getcwd() + '/vendor/re2/', '-I' + os.getcwd() + "/tora/", '-march=native', '-g',
            # '-DPERLISH_CLOSURE'
        ],
        PREFIX=GetOption('prefix'),
        tools=tools
    )
    re2_env = Environment(
        ENV={'PATH': os.environ['PATH']},
        CCFLAGS=['-Wno-sign-compare', '-O2', '-I./vendor/re2/'],
        LIBS=['pthread'],
        tools=tools
    )
    env.MergeFlags([
        '-licuuc48 -licudt48'
    ])
else:
    env = Environment(
        LIBS=['re2', 'pthread', 'dl', 'icudata', 'icuuc'],
        LIBPATH=['./'],
        CXXFLAGS=['-std=c++0x'],
        CPPPATH=[os.path.abspath(x) for x in ['vendor/boost_1_49_0/', 'vendor/re2/', 'tora/']],
        CCFLAGS=[
            '-Wall',
            '-Wno-sign-compare',
            '-fstack-protector',
            '-g',
            '-fPIC',
            # '-DPERLISH_CLOSURE'
            # '-march=native', 
        ],
        PREFIX=GetOption('prefix'),
        tools=tools
    )
    re2_env = Environment(
        CCFLAGS=[
            '-pthread',
            '-Wno-sign-compare',
            '-O2',
            '-I./vendor/re2/',
            '-fPIC'
        ],
        CPPPATH=['vendor/re2/'],
        LIBS=['pthread'],
        tools=tools
    )
    icu_config = 'icu-config'
    if os.uname()[0] == 'Darwin':
        # workaround for homebrew's issue.
        # see https://github.com/mxcl/homebrew/issues/issue/167
        files = Glob('/usr/local/Cellar/icu4c/*/bin/icu-config')
        if len(files) > 0:
            icu_config = files[-1]
    env.MergeFlags([
        '!%s --cppflags --ldflags' % (icu_config)
    ])

env.Append(TORA_LIBPREFIX=env.get('PREFIX') + "/lib/tora-" + TORA_VERSION_STR + "/")

exe_suffix = env.get('PROGSUFFIX')
ext_suffix = env.get('SHLIBSUFFIX')
lemon = os.name == 'nt' and '.\\tools\\lemon\\lemon.exe' or './tools/lemon/lemon'

if os.name == 'nt':
    #re2_env.Replace(CXX='g++')
    #env.Replace(CXX='g++')
    #env.Append(
    #    LINKFLAGS=['-Wl,-E'],
    #)
    pass
elif os.uname()[0]=='Darwin':
    re2_env.Replace(CXX='clang++', CC='clang')
    env.Replace(CXX='clang++', CC='clang')
    # env.Append(CXXFLAGS=['-Werror'])
    env.Append(
        CCFLAGS=['-Wno-unused-function', '-DBOOST_NO_CHAR16_T', '-DBOOST_NO_CHAR32_T',
            '-arch', 'x86_64',
        ],
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
        symbol_table.cc frame.cc operator.cc
        builtin.cc
        object.cc pad_list.cc
        printf.cc

        ops.gen.cc token.gen.cc lexer.gen.cc vm.gen.cc nodes.gen.cc symbols.gen.cc

        inspector.cc peek.cc disasm.cc

        value/code.cc value/hash.cc value/str.cc value/array.cc value/regexp.cc value/range.cc
        value/object.cc value/int.cc value/bool.cc value/exception.cc
        value/bytes.cc value/class.cc

        object/str.cc object/array.cc object/dir.cc object/stat.cc object/env.cc object/time.cc object/file.cc object/internals.cc object/caller.cc object/code.cc object/symbol.cc
        object/dynaloader.cc object/object.cc object/class.cc
        object/bytes.cc object/regexp.cc object/hash.cc
        object/re2_regexp_matched.cc
        object/int.cc object/double.cc
        object/metaclass.cc object/file_package.cc

    ''')
]

########
# tests.

programs = ['bin/tora' + exe_suffix]
for src in glob("tests/test_*.cc"):
    programs.append(env.Program(src.rstrip(".cc") + '.t' + exe_suffix, [
        libfiles,
        src,
    ]))

prove_prefix = 'PERL5LIB=' + os.path.abspath('util/') + ':' + os.path.abspath('util/Test-Simple-0.98/lib/')  + '$PERL5LIB '
prove_path = 'perl -I ' + os.path.abspath('util/Test-Harness-3.23/lib/') + ' ' + os.path.abspath('util/Test-Harness-3.23/bin/prove')
try:
    os.stat('/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.3/bin/prove') # throws exception if not exists
    prove_path = '/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.3/bin/prove'
except: pass
try:
    os.stat('/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.2/bin/prove')
    prove_path = '/Users/tokuhirom/perl5/perlbrew/perls/perl-5.15.2/bin/prove'
except: pass
prove_command = prove_prefix + ' ' + prove_path
if 'test' in COMMAND_LINE_TARGETS:
    env.Command('test', programs, prove_command + ' --source Tora --source Executable -r tests/ t/tra/*.tra t/tra/*/*.tra --source Perl t')

env.Command('test.valgrind', ['bin/tora' + exe_suffix], 'perl misc/valgrind.pl');

if 'bench' in COMMAND_LINE_TARGETS:
    env.Command('bench', [], 'git log --oneline | head -1 && scons ndebug=1 test && ./bin/tora -V && time ./bin/tora benchmark/fib/fib.tra 39')

if 'op' in COMMAND_LINE_TARGETS:
    env.Command('op', [], 'git log --oneline | head -1 && scons && ./bin/tora -V ; sudo opcontrol --reset; sudo opcontrol --start && time ./bin/tora benchmark/fib/fib.tra 39 ; sudo opcontrol --stop')

env.Command('docs', ['bin/tora' + exe_suffix, Glob("tora/object/*.cc"), Glob("docs/source/*")], './bin/tora -I vendor/Path/lib/ util/docgen.tra && cd docs/ && make html')

########
# main programs

env.Command(['tora/nodes.gen.h', 'tora/nodes.gen.cc'], 'tora/nodes.gen.pl', 'perl tora/nodes.gen.pl > tora/nodes.gen.h')
env.Command(['tora/token.gen.cc', 'tora/token.gen.h'], ['tora/token.gen.pl', 'tora/parser.h'], 'perl tora/token.gen.pl')
env.Command(['tora/lexer.gen.cc'], 'tora/lexer.re', 're2c tora/lexer.re > tora/lexer.gen.cc')
env.Command(['tora/vm.gen.cc', 'tora/ops.gen.h', 'tora/ops.gen.cc'], ['tora/vm.gen.pl', 'vm.inc'], 'perl -I misc/Text-MicroTemplate/ tora/vm.gen.pl')
env.Command(['tora/symbols.gen.cc', 'tora/symbols.gen.h'], ['tora/symbols.gen.pl'], 'perl -I misc/Text-MicroTemplate/ tora/symbols.gen.pl')
t = env.Command(['tora/parser.h', 'tora/parser.cc'], [lemon, 'tora/parser.yy', 'tora/lempar.c'], lemon + ' tora/parser.yy')
Clean(t, 'tora/parser.out')

libre2 = re2_env.Library('re2', re2files)

TORA_PREFIX   = env.get('PREFIX')
TORA_CC       = env.get('CC')
TORA_CXX      = env.get('CXX')
TORA_CCFLAGS  = ' '.join(env.get('CCFLAGS')).replace("\\", "/")
TORA_CXXFLAGS = ' '.join(env.get('CXXFLAGS')).replace("\\", "/")
# config.h
with open('tora/config.h', 'w') as f:
    f.write("#pragma once\n")
    f.write('#define TORA_CC      "' + TORA_CC      + "\"\n")
    f.write('#define TORA_CXX     "' + TORA_CXX     + "\"\n")
    f.write('#define TORA_CCFLAGS "' + TORA_CCFLAGS + "\"\n")
    f.write('#define TORA_PREFIX  "' + TORA_PREFIX + "\"\n")
    f.write('#define TORA_VERSION_STR  "' + TORA_VERSION_STR + "\"\n")

with open('config.json', 'w') as f:
    libs = env.get('LIBS')
    libs += ['tora']
    f.write(json.dumps({
        'PATH':        os.environ['PATH'],
        'CC':          TORA_CC,
        'CXX':         TORA_CXX,
        'CCFLAGS':     env.get('CCFLAGS'),
        'CPPPATH':     env.get('CPPPATH') or [],
        'CXXFLAGS':    env.get('CXXFLAGS'),
        'PREFIX':      env.get('PREFIX'),
        'LIBS':        libs,
        'TOOLS':       tools,
        'SHLIBPREFIX': '',
        'LINKFLAGS':   ['' + x for x in env.get('LINKFLAGS')[1:]],
        'TORA_VERSION_STR': TORA_VERSION_STR,
        'TORA_LIBPREFIX':      env.get('PREFIX') + "/lib/tora-" + TORA_VERSION_STR + "/",
        'TORA_EXESUFFIX':      exe_suffix,
        'TORA_EXTSUFFIX':      ext_suffix,
        'TORA_PROVE':          prove_command,
    }))

with open('lib/Config.tra', 'w') as f:
    f.write("my $Config = {\n")
    f.write("  SHLIBSUFFIX      => '" + env.get("SHLIBSUFFIX") + "',\n")
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

def build_tora():
    libtora = env.Library('tora', [
        libfiles,
        libre2,
    ])
    if os.name == 'nt':
        tora = env.Program('bin/tora' + exe_suffix, [
            ['tora/main.cc'],
            libtora,
            libre2
        ])
        Default(tora)
        return tora
    else:
        tora = env.Program('bin/tora' + exe_suffix, [
            ['tora/main.cc'],
            libtora,
            libre2
        ])
        Default(tora)
        return tora

tora = build_tora()

# lemon
lemon_env = Environment()
lemon_env.Append(CCFLAGS=['-O2'])
lemon_env.Program(lemon, ['tools/lemon/lemon.c']);

# ----------------------------------------------------------------------
# test extensions

def test_exts():
    exts = []
    src = Split('Fcntl JSON Digest-MD5 Socket Path URI-Escape  UV Curl')
    src += Split('HTTP IRC') # depends on Socket
    for ext in src:
        exts += [env.Command(['ext/%s/_tested' % ext], [tora]+exts+Glob('ext/' + ext + '/*'), 'cd ext/%s/ && scons test' % ext)]
    env.Command('test.ext', exts, 'echo ok')
test_exts()

# ----------------------------------------------------------------------
# instalation
installs = []
installs += [env.Install(env['PREFIX']+'/bin/', tora)];
installs+=[env.InstallAs(env['PREFIX']+'/lib/tora-'+TORA_VERSION_STR, 'lib/')]
exts = []
for ext in os.listdir('ext'):
    exts += [env.Command(['ext/%s/_installed' % ext], Glob('ext/' + ext + "/*"), "cd ext/" + ext + " && scons install")]
env.Alias('install', [tora, exts, env['PREFIX']+'/bin/', installs])

