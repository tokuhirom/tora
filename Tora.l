%{
#include <stdio.h>
#include <string.h>
#include "Tora.tab.h"

// yywrap?
int
yywrap(void)
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

%%
