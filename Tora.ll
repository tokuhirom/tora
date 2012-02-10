%{
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include "node.h"
#include "shared_ptr.h"
#include "Tora.tab.hh"

extern "C" int yywrap();

/**
 * string buffer
 */
static std::stringstream *string_buffer = NULL;

static void tora_open_string_literal() {
    if (string_buffer) {
        delete string_buffer;
        string_buffer = NULL;
    }
    string_buffer = new std::stringstream();
}
static void tora_add_string_literal(char c) {
    (*string_buffer) << c;
}
static const char *tora_close_string_literal() {
    const char * ret = strdup(string_buffer->str().c_str());
    delete string_buffer;
    string_buffer = NULL;
    return ret;
}

/**
 * line number
 */
int tora_line_number = 1;
static void increment_line_number() {
    tora_line_number++;
}

// yywrap?
int yywrap(void)
{
    return 1;
}


%}

%start C_COMMENT CC_COMMENT STRING_LITERAL_STATE PERL_COMMENT END REGEXP DIVABLE
%%

<INITIAL,DIVABLE>"+"        return ADD;
<INITIAL,DIVABLE>"-"        return SUBTRACT;
<INITIAL,DIVABLE>"*"        return MUL;
<DIVABLE>"/"        return DIV;
<INITIAL,DIVABLE>"\n"       {
    increment_line_number();
}
<INITIAL,DIVABLE>";" {
    BEGIN INITIAL;
    return SEMICOLON;
}
<INITIAL,DIVABLE>"sub"       return SUB;
<INITIAL,DIVABLE>"for"       return FOR;
<INITIAL,DIVABLE>","       return COMMA;
<INITIAL,DIVABLE>".."       return DOTDOT;
<INITIAL,DIVABLE>"."       return DOT;

<INITIAL,DIVABLE>0 {
    yylval.int_value = 0;
    BEGIN DIVABLE;
    return INT_LITERAL;
}

<INITIAL,DIVABLE>[1-9][0-9]* {
    int tmp;
    sscanf(yytext, "%d", &tmp);
    yylval.int_value = tmp;
    BEGIN DIVABLE;
    return INT_LITERAL;
}
<INITIAL,DIVABLE>[1-9][0-9]*\.[0-9]+ {
    double tmp;
    sscanf(yytext, "%lf", &tmp);
    yylval.double_value = tmp;
    BEGIN DIVABLE;
    return DOUBLE_LITERAL;
}

<INITIAL,DIVABLE>"if" return IF;
<INITIAL,DIVABLE>"<" return GT;
<INITIAL,DIVABLE>"++" return PLUSPLUS;
<INITIAL,DIVABLE>">" return LT;
<INITIAL,DIVABLE>"<=" return GE;
<INITIAL,DIVABLE>">=" return LE;
<INITIAL,DIVABLE>"==" return EQ;
<INITIAL,DIVABLE>"/=" return DIV_ASSIGN;
<INITIAL,DIVABLE>"(" {
    BEGIN INITIAL;
    return L_PAREN;
}
<INITIAL,DIVABLE>")" {
    BEGIN DIVABLE;
    return R_PAREN;
}
<INITIAL,DIVABLE>"{" return L_BRACE;
<INITIAL,DIVABLE>"}" return R_BRACE;
<INITIAL,DIVABLE>"[" return L_BRACKET;
<INITIAL,DIVABLE>"]" {
    BEGIN DIVABLE;
    return R_BRACKET;
}
<INITIAL,DIVABLE>"=" return ASSIGN;
<INITIAL,DIVABLE>"true" return TRUE;
<INITIAL,DIVABLE>"my" return MY;
<INITIAL,DIVABLE>"false" return FALSE;
<INITIAL,DIVABLE>"while" return WHILE;
<INITIAL,DIVABLE>"return" return RETURN;
<INITIAL,DIVABLE>"else" return ELSE;
<INITIAL,DIVABLE>[ \t] ;
<INITIAL,DIVABLE>[ \t\r\n] {
    increment_line_number();
}

<INITIAL,DIVABLE>"\n__END__\n" {
    increment_line_number();
    increment_line_number();
    BEGIN END;
}

<INITIAL,DIVABLE>[A-Za-z_][A-Za-z0-9_]* {
    char *tmp = new char [strlen(yytext)+1];
    strcpy(tmp, yytext);
    yylval.str_value = tmp;
    BEGIN DIVABLE;
    return IDENTIFIER;
}

<INITIAL,DIVABLE>[$][A-Za-z_][A-Za-z0-9_]* {
    char *tmp = new char [strlen(yytext)+1];
    strcpy(tmp, yytext);
    yylval.str_value = tmp;
    BEGIN DIVABLE;
    return VARIABLE;
}

<INITIAL,DIVABLE>\" {
    tora_open_string_literal();
    BEGIN STRING_LITERAL_STATE;
}

<INITIAL,DIVABLE>"/*" {
    /* TODO save last state for DIVABLE? */
    BEGIN C_COMMENT;
}
<INITIAL,DIVABLE>"//" {
    BEGIN CC_COMMENT;
}
<INITIAL,DIVABLE>"#" {
    BEGIN PERL_COMMENT;
}
<INITIAL>"/" {
    tora_open_string_literal();
    BEGIN REGEXP;
}


<C_COMMENT>\n     increment_line_number();
<C_COMMENT>"*/"     {
    BEGIN INITIAL;
}
<C_COMMENT>.      ;
<CC_COMMENT>\n  {
    increment_line_number();
    BEGIN INITIAL;
}
<CC_COMMENT>.   ;
<PERL_COMMENT>\n  {
    increment_line_number();
    BEGIN INITIAL;
}
<PERL_COMMENT>.   ;

<END>. ;
<END>\n ;

<STRING_LITERAL_STATE>\"        {
    yylval.str_value = tora_close_string_literal();
    BEGIN INITIAL;
    return STRING_LITERAL;
}
<STRING_LITERAL_STATE>\n        {
    tora_add_string_literal('\n');
    increment_line_number();
}
<STRING_LITERAL_STATE>\\\"      tora_add_string_literal('"');
<STRING_LITERAL_STATE>\\n       tora_add_string_literal('\n');
<STRING_LITERAL_STATE>\\t       tora_add_string_literal('\t');
<STRING_LITERAL_STATE>\\\\      tora_add_string_literal('\\');
<STRING_LITERAL_STATE>[\x81-\x9f\xe0-\xef][\x40-\x7e\x80-\xfc] {
  tora_add_string_literal(yytext[0]);
  tora_add_string_literal(yytext[1]);
}
<STRING_LITERAL_STATE>.         tora_add_string_literal(yytext[0]);

<REGEXP>[/] {
    // # <REGEXP>"\\/" tora_add_string_literal('/');
    /* TODO: options like /xsmi */
    yylval.str_value = tora_close_string_literal();
    BEGIN INITIAL;
    return REGEXP_LITERAL;
}
<REGEXP>. {
    tora_add_string_literal(yytext[0]);
}

%%
