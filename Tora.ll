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

%start C_COMMENT CC_COMMENT STRING_LITERAL_STATE PERL_COMMENT END
%%

<INITIAL>"+"        return ADD;
<INITIAL>"-"        return SUBTRACT;
<INITIAL>"*"        return MUL;
<INITIAL>"/"        return DIV;
<INITIAL>"\n"       {
    increment_line_number();
}
<INITIAL>";"       return SEMICOLON;
<INITIAL>"sub"       return SUB;
<INITIAL>"for"       return FOR;
<INITIAL>","       return COMMA;
<INITIAL>".."       return DOTDOT;
<INITIAL>"."       return DOT;

<INITIAL>0 {
    yylval.int_value = 0;
    return INT_LITERAL;
}

<INITIAL>[1-9][0-9]* {
    int tmp;
    sscanf(yytext, "%d", &tmp);
    yylval.int_value = tmp;
    return INT_LITERAL;
}
<INITIAL>[1-9][0-9]*\.[0-9]+ {
    double tmp;
    sscanf(yytext, "%lf", &tmp);
    yylval.double_value = tmp;
    return DOUBLE_LITERAL;
}

<INITIAL>"if" return IF;
<INITIAL>"<" return GT;
<INITIAL>">" return LT;
<INITIAL>"<=" return GE;
<INITIAL>">=" return LE;
<INITIAL>"==" return EQ;
<INITIAL>"(" return L_PAREN;
<INITIAL>")" return R_PAREN;
<INITIAL>"{" return L_BRACE;
<INITIAL>"}" return R_BRACE;
<INITIAL>"[" return L_BRACKET;
<INITIAL>"]" return R_BRACKET;
<INITIAL>"=" return ASSIGN;
<INITIAL>"true" return TRUE;
<INITIAL>"my" return MY;
<INITIAL>"false" return FALSE;
<INITIAL>"while" return WHILE;
<INITIAL>"return" return RETURN;
<INITIAL>"else" return ELSE;
<INITIAL>[ \t] ;
<INITIAL>[ \t\r\n] {
    increment_line_number();
}

<INITIAL>"\n__END__\n" {
    increment_line_number();
    increment_line_number();
    BEGIN END;
}

<INITIAL>[A-Za-z_][A-Za-z0-9_]* {
    char *tmp = new char [strlen(yytext)+1];
    strcpy(tmp, yytext);
    yylval.str_value = tmp;
    return IDENTIFIER;
}

<INITIAL>[$][A-Za-z_][A-Za-z0-9_]* {
    char *tmp = new char [strlen(yytext)+1];
    strcpy(tmp, yytext);
    yylval.str_value = tmp;
    return VARIABLE;
}

<INITIAL>\" {
    tora_open_string_literal();
    BEGIN STRING_LITERAL_STATE;
}

<INITIAL>"/*" {
    BEGIN C_COMMENT;
}
<INITIAL>"//" {
    BEGIN CC_COMMENT;
}
<INITIAL>"#" {
    BEGIN PERL_COMMENT;
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

%%
