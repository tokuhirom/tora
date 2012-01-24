%{
#include <stdio.h>
#include <stdlib.h>
#define YYDEBUG 1

%}
%token IF
%union {
    int int_value;
}

%token <int_value> INT_LITERAL;
%token ADD SUB MUL DIV CR
%type <int_value> expression term primary_expression

%%

line_list
    : line
    | line_list line
    ;

line
    :expression CR
    {
        printf(">> %d\n", $1);
    }

expression
    : term
    | expression ADD term
    {
        $$ = $1 + $3
    }
    | expression SUB term
    {
        $$ = $1 - $3
    }
    ;

term
    : primary_expression
    | term MUL primary_expression
    {
        $$ = $1 * $3;
    }
    | term DIV primary_expression
    {
        $$ = $1 / $3;
    }
    ;

primary_expression
    : INT_LITERAL
    ;

%%
int yyerror(const char *err) {
    extern char *yytext;
    fprintf(stderr, "parser error near %s\n", yytext);
    return 0;
}

int main() {
    extern int yyparse(void);
    extern FILE *yyin;
    yyin = stdin;
    if (yyparse()) {
        fprintf(stderr, "Error!\n");
        exit(1);
    }
}

