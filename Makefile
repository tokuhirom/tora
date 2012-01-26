COPTS=-Wall

all: tora

tora: Tora.tab.o lex.yy.o main.o
	g++ $(COPTS) -g -o tora Tora.tab.o lex.yy.o main.o vm.cc

lex.yy.o: lex.yy.c
	g++ $(COPTS) -g -o lex.yy.o -c lex.yy.c

ops.gen.h: ops.gen.pl
	perl ops.gen.pl

main.o: main.cc
	g++ $(COPTS) -g -o main.o -c main.cc

Tora.tab.o: Tora.tab.cc
	g++ $(COPTS) -g -o Tora.tab.o -c Tora.tab.cc

Tora.tab.cc: Tora.yy ops.gen.h
	bison -dv Tora.yy

lex.yy.c: Tora.l
	flex Tora.l

clean:
	rm -f lex.yy.cc lex.yy.o Tora.tab.cc Tora.output Tora.tab.hh tora Tora.tab.o lex.yy.c

test: tora
	prove t/

.PHONY: all clean test

