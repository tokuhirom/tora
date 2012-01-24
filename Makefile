all: tora

tora: Tora.tab.c lex.yy.c
	gcc -g -o tora Tora.tab.c lex.yy.c

Tora.tab.c: Tora.y
	bison -dv Tora.y

lex.yy.c: Tora.l
	flex Tora.l

clean:
	rm -f lex.yy.c Tora.tab.c Tora.output Tora.tab.h tora

.PHONY: all clean

