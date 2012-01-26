%{
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <cassert>
#include <sstream>
#include <map>
#include <unistd.h>

#include "ops.gen.h"
#include "tora.h"
#include "value.h"
#include "vm.h"

#define YYDEBUG 1

extern int yylex();
int yyerror(const char *err);

static TNode *tora_create_binary_expression(NODE_TYPE type, TNode *t1, TNode* t2) {
    TNode *node = new TNode();
    node->type = type;
    node->binary.left  = t1;
    node->binary.right = t2;
    return node;
}

void tora_dump_node(TNode *node, int indent) {
    for (int i=0; i<indent*4; i++) {
        printf(" ");
    }
    switch (node->type) {
    case NODE_STRING:
        printf("NODE_STRING('%s')\n", node->str_value);
        break;
    case NODE_INT:
        printf("NODE_INT(%d)\n", node->int_value);
        break;
    case NODE_TRUE:
        printf("NODE_TRUE\n");
        break;
    case NODE_FALSE:
        printf("NODE_FALSE\n");
        break;
    case NODE_IDENTIFIER:
        printf("NODE_IDENTIFIER(%s)\n", node->str_value);
        break;
    case NODE_NEWLINE:
        printf("NODE_NEWLINE\n");
        break;

    case NODE_FUNCALL:
        printf("NODE_FUNCALL\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_ADD:
        printf("NODE_ADD\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_SUB:
        printf("NODE_SUB\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_MUL:
        printf("NODE_MUL\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_DIV:
        printf("NODE_DIV\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_STMTS:
        printf("NODE_STMTS\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_IF:
        printf("NODE_IF\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;

    default:
        printf("Unknown node\n");
        break;
    }
}
void tora_dump_node(TNode *node) {
    tora_dump_node(node, 0);
}

TNode *root_node;

%}
%token IF
%token L_PAREN R_PAREN L_BRACE R_BRACE
%union {
    int int_value;
    char *str_value;
    struct TNode *node;
}

%token <int_value> INT_LITERAL;
%token <str_value> IDENTIFIER;
%token <str_value> VARIABLE;
%token ADD SUB MUL DIV CR
%token FOR WHILE
%token TRUE FALSE
%token LT GT LE GE EQ
%token ASSIGN
%token MY
%token <str_value>STRING_LITERAL
%type <node> expression2 expression3 expression term primary_expression line line_list root lvalue variable block
%type <node> identifier

%%

root
    : line_list
    {
        // tora_dump_node($1);
        root_node = $1;
    }
    ;

line_list
    : line
    | line_list line
    {
        $$ = tora_create_binary_expression(NODE_STMTS, $1, $2);
    }
    ;

line
    :expression CR
    {
        $$ = $1;
    }
    | CR
    {
        TNode *node = new TNode();
        node->type = NODE_NEWLINE;
        $$ = node;
    }
    | IF L_PAREN expression R_PAREN block
    {
        $$ = tora_create_binary_expression(NODE_IF, $3, $5);
    }
    | WHILE L_PAREN expression R_PAREN block
    {
        $$ = tora_create_binary_expression(NODE_WHILE, $3, $5);
    }
    | lvalue ASSIGN expression
    {
        $$ = tora_create_binary_expression(NODE_ASSIGN, $1, $3);
    }
    | identifier L_PAREN expression R_PAREN
    {
        // funciton call
        // TODO: support multiple args
        // TODO: support vargs
        // call function
        $$ = tora_create_binary_expression(NODE_FUNCALL, $1, $3);
    }
    ;

block
    : L_BRACE line_list R_BRACE
    {
        // TODO: enterscope
        // TODO: leavescope
        $$ = $2;
    }

lvalue
    : variable
    ;

expression
    : expression2
    | expression EQ expression2
    {
        $$ = tora_create_binary_expression(NODE_EQ, $1, $3);
    }
    ;

expression2
    : expression3
    | expression2 LT expression3
    {
        $$ = tora_create_binary_expression(NODE_LT, $1, $3);
    }
    | expression2 GT expression3
    {
        $$ = tora_create_binary_expression(NODE_GT, $1, $3);
    }
    | expression2 LE expression3
    {
        $$ = tora_create_binary_expression(NODE_LE, $1, $3);
    }
    | expression2 GE expression3
    {
        $$ = tora_create_binary_expression(NODE_GE, $1, $3);
    }
    ;

expression3
    : term
    | expression ADD term
    {
        $$ = tora_create_binary_expression(NODE_ADD, $1, $3);
    }
    | expression SUB term
    {
        $$ = tora_create_binary_expression(NODE_SUB, $1, $3);
    }
    ;

term
    : primary_expression
    | term MUL primary_expression
    {
        $$ = tora_create_binary_expression(NODE_MUL, $1, $3);
    }
    | term DIV primary_expression
    {
        $$ = tora_create_binary_expression(NODE_DIV, $1, $3);
    }
    ;

primary_expression
    : INT_LITERAL
    {
        TNode *node = new TNode();
        node->type = NODE_INT;
        node->int_value = $1;
        $$ = node;
    }
    | FALSE
    {
        TNode *node = new TNode();
        node->type = NODE_FALSE;
        $$ = node;
    }
    | TRUE
    {
        TNode *node = new TNode();
        node->type = NODE_TRUE;
        $$ = node;
    }
    | STRING_LITERAL
    {
        TNode *node = new TNode();
        node->type = NODE_STRING;
        node->str_value = $1;
        $$ = node;
    }
    | variable
    ;

identifier
    : IDENTIFIER
    {
        TNode *node = new TNode();
        node->type = NODE_IDENTIFIER;
        node->str_value = $1;
        $$ = node;
    }
    ;

variable
    : VARIABLE
    {
        TNode *node = new TNode();
        node->type = NODE_VARIABLE;
        node->str_value = $1;
        $$ = node;
    }
    ;

%%

int yyerror(const char *err) {
    extern char *yytext;
    fprintf(stderr, "parser error near %s\n", yytext);
    return 0;
}


