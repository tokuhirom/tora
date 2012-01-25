all: tora

tora: Tora.tab.o lex.yy.o
	g++ -g -o tora Tora.tab.o lex.yy.o

lex.yy.o: lex.yy.c
	g++ -g -o lex.yy.o -c lex.yy.c

Tora.tab.o: Tora.tab.cc
	g++ -g -o Tora.tab.o -c Tora.tab.cc

Tora.tab.cc: Tora.yy
	bison -dv Tora.yy

lex.yy.c: Tora.l
	flex Tora.l

clean:
	rm -f lex.yy.cc lex.yy.o Tora.tab.cc Tora.output Tora.tab.hh tora Tora.tab.o lex.yy.c

test: tora
	prove t/

.PHONY: all clean test

