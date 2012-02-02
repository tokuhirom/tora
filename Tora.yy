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

static TNode *tora_create_root_node(TNode *t) {
    TNode *node = new TNode();
    node->type = NODE_ROOT;
    node->node = t;
    return node;
}

static TNode *tora_create_my(TNode *t) {
    TNode *node = new TNode();
    node->type = NODE_MY;
    node->node = t;
    return node;
}

static TNode *tora_create_block(TNode *t) {
    TNode *node = new TNode();
    node->type = NODE_BLOCK;
    node->node = t;
    return node;
}

static TNode *tora_create_funcdef(TNode *name, std::vector<TNode *>*params, TNode *block) {
    TNode *node = new TNode();
    node->type = NODE_FUNCDEF;
    node->funcdef.name   = name;
    node->funcdef.params = params;
    node->funcdef.block  = block;
    return node;
}

static TNode *tora_create_return(TNode *child) {
    TNode *node = new TNode();
    node->type = NODE_RETURN;
    node->node = child;
    return node;
}

static TNode *tora_create_void() {
    TNode *node = new TNode();
    node->type = NODE_VOID;
    return node;
}

static TNode *tora_create_binary_expression(NODE_TYPE type, TNode *t1, TNode* t2) {
    TNode *node = new TNode();
    node->type = type;
    node->binary.left  = t1;
    node->binary.right = t2;
    return node;
}

static TNode *tora_create_if(TNode *cond, TNode* if_body, TNode *else_body) {
    TNode *node = new TNode();
    node->type = NODE_IF;
    node->if_stmt.cond      = cond;
    node->if_stmt.if_body   = if_body;
    node->if_stmt.else_body = else_body;
    return node;
}

static TNode *tora_create_funcall(TNode *t1, std::vector<struct TNode*> *args) {
    TNode *node = new TNode();
    node->type = NODE_FUNCALL;
    node->funcall.name  = t1;
    node->funcall.args  = args;
    return node;
}


TNode *root_node;

typedef std::vector<struct TNode*> argument_list_t;

%}
%token IF ELSE
%token L_PAREN R_PAREN L_BRACE R_BRACE
%union {
    int int_value;
    char *str_value;
    struct TNode *node;
    std::vector<struct TNode*> *argument_list;
    std::vector<struct TNode*> *parameter_list;
}

%token <int_value> INT_LITERAL;
%token <str_value> IDENTIFIER;
%token <str_value> VARIABLE;
%token ADD SUBTRACT MUL DIV CR
%token FOR WHILE
%token TRUE FALSE
%token LT GT LE GE EQ
%token ASSIGN
%token MY
%token COMMA RETURN SEMICOLON
%token L_BRACKET R_BRACKET
%token <str_value>STRING_LITERAL
%token SUB
%type <node> expression2 expression3 expression term primary_expression line line_list root variable block postfix_expression void sub_stmt if_statement
%type <node> identifier
%type <argument_list> argument_list
%type <parameter_list> parameter_list

%%

root
    : line_list
    {
        root_node = tora_create_root_node($1);
    }
    ;

void:
    /* empty */

line_list
    : line
    | line_list line
    {
        $$ = tora_create_binary_expression(NODE_STMTS, $1, $2);
    }
    | expression
    | void
    {
        $$ = tora_create_void();
    }
    ;

line
    :expression SEMICOLON
    {
        $$ = $1;
    }
    |expression CR
    {
        $$ = $1;
    }
    | RETURN expression
    {
        $$ = tora_create_return($2);
    }
    | CR
    {
        TNode *node = new TNode();
        node->type = NODE_NEWLINE;
        $$ = node;
    }
    | if_statement
    | MY variable ASSIGN expression
    {
        TNode *n1 = tora_create_my($2);
        TNode *node = new TNode();
        node->type = NODE_SETVARIABLE;
        node->set_value.lvalue = $2;
        node->set_value.rvalue = $4;
        $$ = tora_create_binary_expression(NODE_STMTS, n1, node);
    }
    | variable ASSIGN expression
    {
        TNode *node = new TNode();
        node->type = NODE_SETVARIABLE;
        node->set_value.lvalue = $1;
        node->set_value.rvalue = $3;
        $$ = node;
    }
    | WHILE L_PAREN expression R_PAREN block
    {
        $$ = tora_create_binary_expression(NODE_WHILE, $3, $5);
    }
    | sub_stmt
    | block
    ;

if_statement
    : IF L_PAREN expression R_PAREN block
    {
        $$ = tora_create_if($3, $5, NULL);
    }
    | IF L_PAREN expression R_PAREN block ELSE block
    {
        $$ = tora_create_if($3, $5, $7);
    }

sub_stmt
    : SUB identifier L_PAREN parameter_list R_PAREN block
    {
        $$ = tora_create_funcdef($2, $4, $6);
    }

parameter_list
    : variable
    {
        $$ = new std::vector<TNode*>();
        $$->push_back($1);
    }
    | parameter_list COMMA expression
    {
        $$->push_back($3);
    }
    | void
    {
        $$ = new std::vector<TNode*>();
    }

argument_list
    : expression
    {
        $$ = new std::vector<TNode*>();
        $$->push_back($1);
    }
    | argument_list COMMA expression
    {
        $$->push_back($3);
    }
    ;

block
    : L_BRACE line_list R_BRACE
    {
        $$ = tora_create_block($2);
    }

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
    | expression SUBTRACT term
    {
        $$ = tora_create_binary_expression(NODE_SUB, $1, $3);
    }
    ;

term
    : postfix_expression
    | term MUL primary_expression
    {
        $$ = tora_create_binary_expression(NODE_MUL, $1, $3);
    }
    | term DIV primary_expression
    {
        $$ = tora_create_binary_expression(NODE_DIV, $1, $3);
    }
    ;

postfix_expression
    : primary_expression
    | identifier L_PAREN R_PAREN
    {
        $$ = tora_create_funcall($1, new std::vector<TNode*>());
    }
    | identifier L_PAREN argument_list R_PAREN
    {
        // TODO: support vargs
        $$ = tora_create_funcall($1, $3);
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
        node->type = NODE_GETVARIABLE;
        node->str_value = $1;
        $$ = node;
    }
    ;

%%

int yyerror(const char *err) {
    extern char *yytext;
    extern int tora_line_number;
    fprintf(stderr, "parser error near %s at line %d\n", yytext, tora_line_number);
    return 0;
}


