COPTS=-Wall -g -std=c++0x
CXX=g++

all: tora

tora: Tora.tab.o lex.yy.o main.o value.o compiler.o vm.o
	$(CXX) -g -o tora Tora.tab.o lex.yy.o main.o vm.o value.o compiler.o

ops.gen.h: ops.gen.pl
	perl ops.gen.pl

lex.yy.o: lex.yy.c
	$(CXX) $(COPTS) -c -o $@ $<

.cc.o:
	$(CXX) $(COPTS) -c -o $@ $<

Tora.tab.cc: Tora.yy ops.gen.h
	bison -dv Tora.yy

lex.yy.c: Tora.l
	flex Tora.l

clean:
	rm -f lex.yy.cc lex.yy.o Tora.tab.cc Tora.output Tora.tab.hh tora Tora.tab.o lex.yy.c main.o ops.gen.h value.o

test: tora
	prove t/

.PHONY: all clean test

