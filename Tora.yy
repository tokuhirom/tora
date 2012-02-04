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
#include "nodes.gen.h"
#include "node.h"
#include "tora.h"
#include "value.h"
#include "vm.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

using namespace tora;

extern int yylex();
int yyerror(const char *err);

static Node *tora_create_funcdef(Node *name, std::vector<Node *>*params, Node *block) {
    return new FuncdefNode(name, params, block);
}

static Node *tora_create_return(Node *child) {
    return new NodeNode(NODE_RETURN, child);
}

static Node *tora_create_binary_expression(node_type_t type, Node *t1, Node* t2) {
    return new BinaryNode(type, t1, t2);
}

static Node *tora_create_unary_expression(node_type_t type, Node *t1) {
    return new NodeNode(type, t1);
}

static Node *tora_create_if(Node *cond, Node* if_body, Node *else_body) {
    return new IfNode(NODE_IF, cond, if_body, else_body);
}

static Node *tora_create_funcall(Node *t1, std::vector<Node*> *args) {
    return new FuncallNode(t1, args);
}


Node *root_node;

// see http://www.lysator.liu.se/c/ANSI-C-grammar-y.html

%}

// TODO: reentrant
// %pure_parser

%union {
    int int_value;
    const char *str_value;
    struct Node *node;
    std::vector<struct Node*> *argument_list;
    std::vector<struct Node*> *parameter_list;
}

%type <node> additive_expression multiplicative_expression primary_expression variable block postfix_expression sub_stmt if_statement array_creation unary_expression jump_statement translation_unit statement root statement_list inclusive_or_expression exclusive_or_expression and_expression
%type <node> expression
%type <node> assignment_expression
%type <node> conditional_expression
%type <node> logical_or_expression logical_and_expression
%type <node> equality_expression
%type <node> relational_expression
%type <node> shift_expression
%type <node> identifier
%type <argument_list> argument_list
%type <parameter_list> parameter_list

%token IF ELSE
%token L_PAREN R_PAREN L_BRACE R_BRACE
%token <int_value> INT_LITERAL;
%token <str_value> IDENTIFIER;
%token <str_value> VARIABLE;
%token <str_value>STRING_LITERAL
%token FOR WHILE
%nonassoc TRUE FALSE
%right ASSIGN
%right MY
%token COMMA RETURN SEMICOLON
%token SUB

%left DOT
%left DOTDOT
%left L_BRACKET R_BRACKET
%left EQ
%left LT GT LE GE
%left ADD SUBTRACT
%left MUL DIV
%right '!'        /* 右結合で優先順位1 */

%%

root
    : translation_unit
    {
        root_node = new NodeNode(NODE_ROOT, $1);
    }
    |
    {
        root_node = new NodeNode(NODE_ROOT, new VoidNode(NODE_VOID));
    }

translation_unit
    : statement
    | translation_unit statement
    {
         $$ = tora_create_binary_expression(NODE_STMTS, $1, $2);
    }
    ;

statement
    :expression SEMICOLON
    {
        $$ = $1;
    }
    | SEMICOLON
    {
        $$ = new VoidNode(NODE_VOID);
    }
    | jump_statement
    | if_statement
    | WHILE L_PAREN expression R_PAREN block
    {
        $$ = tora_create_binary_expression(NODE_WHILE, $3, $5);
    }
    | FOR L_PAREN expression SEMICOLON expression SEMICOLON expression R_PAREN block
    {
        $$ = new ForNode($3, $5, $7, $9);
    }
    | sub_stmt
    | block
    ;

jump_statement
    : RETURN expression SEMICOLON
    {
        $$ = tora_create_return($2);
    }

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
    | SUB identifier L_PAREN R_PAREN block
    {
        $$ = tora_create_funcdef($2, new std::vector<Node*>(), $5);
    }

parameter_list
    : variable
    {
        $$ = new std::vector<Node*>();
        $$->push_back($1);
    }
    | parameter_list COMMA expression
    {
        $$->push_back($3);
    }

/* array constructor */
array_creation
    : L_BRACKET argument_list R_BRACKET
    {
        $$ = new ArgsNode(NODE_MAKE_ARRAY, $2);
    }

argument_list
    : expression
    {
        $$ = new std::vector<Node*>();
        $$->push_back($1);
    }
    | argument_list COMMA expression
    {
        $$->push_back($3);
    }
    ;

block
    : L_BRACE statement_list R_BRACE
    {
        $$ = new NodeNode(NODE_BLOCK, $2);
    }
    | L_BRACE R_BRACE
    {
        $$ = new VoidNode(NODE_VOID);
    }

statement_list
    : statement_list statement
    {
        $$ = tora_create_binary_expression(NODE_STMTS, $1, $2);
    }
    | statement

expression
    : assignment_expression

assignment_expression
    : conditional_expression
    | MY variable ASSIGN expression
    {
        // REMOVE THIS NODE
        Node *n1 = new NodeNode(NODE_MY, $2);
        Node *node = new BinaryNode(NODE_SETVARIABLE, $2, $4);
        $$ = tora_create_binary_expression(NODE_STMTS, n1, node);
    }
    | conditional_expression ASSIGN conditional_expression
    {
        $$ = new BinaryNode(NODE_SETVARIABLE, $1, $3);
    }

conditional_expression
    : logical_or_expression

logical_or_expression
    : logical_and_expression

logical_and_expression
    : inclusive_or_expression

inclusive_or_expression
    : exclusive_or_expression

exclusive_or_expression
    : and_expression

and_expression
    : equality_expression

equality_expression
    : relational_expression
    | equality_expression EQ relational_expression
    {
        $$ = tora_create_binary_expression(NODE_EQ, $1, $3);
    }
    ;

relational_expression
    : shift_expression
    | relational_expression LT shift_expression
    {
        $$ = tora_create_binary_expression(NODE_LT, $1, $3);
    }
    | relational_expression GT shift_expression
    {
        $$ = tora_create_binary_expression(NODE_GT, $1, $3);
    }
    | relational_expression LE shift_expression
    {
        $$ = tora_create_binary_expression(NODE_LE, $1, $3);
    }
    | relational_expression GE shift_expression
    {
        $$ = tora_create_binary_expression(NODE_GE, $1, $3);
    }
    ;

shift_expression
    : additive_expression

additive_expression
    : multiplicative_expression
    | additive_expression ADD multiplicative_expression
    {
        $$ = tora_create_binary_expression(NODE_ADD, $1, $3);
    }
    | additive_expression SUBTRACT multiplicative_expression
    {
        $$ = tora_create_binary_expression(NODE_SUB, $1, $3);
    }
    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression MUL unary_expression
    {
        $$ = tora_create_binary_expression(NODE_MUL, $1, $3);
    }
    | multiplicative_expression DIV unary_expression
    {
        $$ = tora_create_binary_expression(NODE_DIV, $1, $3);
    }
    ;

unary_expression
    : postfix_expression
    | SUBTRACT unary_expression
    {
        $$ = tora_create_unary_expression(NODE_UNARY_NEGATIVE, $2);
    }

postfix_expression
    : primary_expression
    | primary_expression L_BRACKET expression R_BRACKET
    {
        $$ = new BinaryNode(NODE_GET_ITEM, $1, $3);
    }
    | identifier L_PAREN R_PAREN
    {
        $$ = tora_create_funcall($1, new std::vector<Node*>());
    }
    | identifier L_PAREN argument_list R_PAREN
    {
        // TODO: support vargs
        $$ = tora_create_funcall($1, $3);
    }
    | primary_expression DOT identifier L_PAREN argument_list R_PAREN
    {
        $$ = new MethodCallNode($1, $3, $5);
    }
    | primary_expression DOT identifier L_PAREN R_PAREN
    {
        $$ = new MethodCallNode($1, $3, new std::vector<Node*>());
    }
    ;

primary_expression
    : INT_LITERAL
    {
        $$ = new IntNode(NODE_INT, $1);
    }
    | FALSE
    {
        $$ = new VoidNode(NODE_FALSE);
    }
    | TRUE
    {
        $$ = new VoidNode(NODE_TRUE);
    }
    | STRING_LITERAL
    {
        $$ = new StrNode(NODE_STRING, $1);
    }
    | variable
    | array_creation
    | L_PAREN expression R_PAREN
    {
        $$ = $2;
    }
    ;

identifier
    : IDENTIFIER
    {
        $$ = new StrNode(NODE_IDENTIFIER, $1);
    }
    ;

variable
    : VARIABLE
    {
        $$ = new StrNode(NODE_GETVARIABLE, $1);
    }
    ;

%%

int yyerror(const char *err) {
    extern char *yytext;
    extern int tora_line_number;
    fprintf(stderr, "%s near '%s' at line %d\n", err, yytext, tora_line_number);
    return 0;
}


