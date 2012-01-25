%{
#include <stdio.h>
#include <string.h>
#include "Tora.tab.hh"

extern "C" int yywrap();

// yywrap?
int yywrap(void)
{
    return 1;
}


%}
%%

"+"        return ADD;
"-"        return SUB;
"*"        return MUL;
"/"        return DIV;
"\n"       return CR;

[1-9][0-9]* {
    int tmp;
    sscanf(yytext, "%d", &tmp);
    yylval.int_value = tmp;
    return INT_LITERAL;
}

"if" return IF;
"(" return L_PAREN;
")" return R_PAREN;
"{" return L_BRACE;
"}" return R_BRACE;
"true" return TRUE;
"false" return FALSE;
[ \t] ;
[\n] {
    // increment_line_number();
    return CR;
}

[A-Za-z_][A-Za-z0-9_]* {
    char *tmp = new char [strlen(yytext)+1];
    strcpy(tmp, yytext);
    printf("GET IDENTIFIER in LEX: '%s'\n", tmp);
    yylval.str_value = tmp;
    return IDENTIFIER;
}

%%
