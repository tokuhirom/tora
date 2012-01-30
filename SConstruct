import os
import subprocess

perl_builder = Builder(action = 'perl < $SOURCE > $TARGET')

if 'test' in COMMAND_LINE_TARGETS:
    subprocess.call(['prove', '-r', 't']);
    exit()

env = Environment(BUILDERS={"Perl":perl_builder})
env.Append(CCFLAGS=['-g'])
env.CXXFile(target='Tora.tab.cc', source='Tora.yy', YACCFLAGS='-dv')
env.Perl(target='ops.gen.h', source='ops.gen.pl')
env.CXXFile(target='lex.yy.c', source='Tora.ll')
env.Program('tora', ['Tora.tab.cc lex.yy.c main.cc value.cc compiler.cc vm.cc'.split(' ')], CXXFLAGS='--std=c++0x', CC='g++')
