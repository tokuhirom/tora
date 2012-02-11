%right ASSIGN.
%right MY.
%left DIV_ASSIGN.
%left PLUSPLUS.
%left DOT.
%left DOTDOT.
%left L_BRACKET R_BRACKET.
%left EQ.
%left LT GT LE GE.
%left ADD SUB.
%left MUL DIV.
/* %right '!'. */

%token_type { YYSTYPE }

%include {
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
#include "shared_ptr.h"
#include "regexp.h"
#include "parser.h"
#include "lexer.h"

using namespace tora;

// see http://www.lysator.liu.se/c/ANSI-C-grammar-y.html

}

%parse_failure {
    state->failure = true;
    fprintf(stderr, "Syntax error at line %d.\n", state->lineno);
}
%syntax_error {
    state->errors++;
    fprintf(stderr, "Syntax error at line %d.\n", state->lineno);
}

%extra_argument { ParserState *state }

%stack_overflow {
    state->errors++;
    fprintf(stderr,"Giving up.  Parser stack overflow?n");
}

%start_symbol root

root ::= translation_unit(A). {
    state->root_node = new NodeNode(NODE_ROOT, A.node);
}
root ::= . {
    state->root_node = new NodeNode(NODE_ROOT, new VoidNode(NODE_VOID));
}

translation_unit(A) ::= statement(B). { A.node = B.node; }
translation_unit(A) ::= translation_unit(B) statement(C). {
    A.node = new BinaryNode(NODE_STMTS, B.node, C.node);
}

statement(A) ::= expression(B) SEMICOLON . {
    A.node = B.node;
}
statement(A) ::= SEMICOLON. {
    A.node = new VoidNode(NODE_VOID);
}
statement(A) ::= jump_statement(B). { A.node = B.node; }
statement(A) ::= if_statement(B).   { A.node = B.node; }
statement(A) ::= WHILE L_PAREN expression(B) R_PAREN block(C). {
    A.node = new BinaryNode(NODE_WHILE, B.node, C.node);
}
statement(A) ::= FOR L_PAREN expression(B) SEMICOLON expression(C) SEMICOLON expression(D) R_PAREN block(E). {
    A.node = new ForNode(B.node, C.node, D.node, E.node);
}
statement(A) ::= sub_stmt(B).   { A.node = B.node; }
statement(A) ::= block(B).   { A.node = B.node; }

jump_statement(A) ::= RETURN expression(B) SEMICOLON. {
    A.node = new NodeNode(NODE_RETURN, B.node);
}

if_statement(A) ::= IF L_PAREN expression(B) R_PAREN block(C). {
    A.node = new IfNode(NODE_IF, B.node, C.node, NULL);
}
if_statement(A) ::= IF L_PAREN expression(B) R_PAREN block(C) ELSE block(D). {
    A.node = new IfNode(NODE_IF, B.node, C.node, D.node);
}

sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN parameter_list(C) R_PAREN block(D). {
    A.node = new FuncdefNode(B.node, C.node_list, D.node);
}
sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN R_PAREN block(C). {
    A.node = new FuncdefNode(B.node, new std::vector<SharedPtr<Node>>(), C.node);
}

parameter_list(A) ::= variable(B). {
    A.node_list = new std::vector<SharedPtr<Node>>();
    A.node_list->push_back(B.node);
}
parameter_list(A) ::= parameter_list(B) COMMA expression(C). {
    A.node_list = B.node_list;
    A.node_list->push_back(C.node);
}

/* array constructor [a, b] */
array_creation(A) ::= L_BRACKET argument_list(B) R_BRACKET. {
    A.node = new ArgsNode(NODE_MAKE_ARRAY, B.node_list);
}

argument_list(A) ::= expression(B). {
    A.node_list = new std::vector<SharedPtr<Node>>();
    A.node_list->push_back(B.node);
}
argument_list(A) ::= argument_list(B) COMMA expression(C). {
    A.node_list = B.node_list;
    A.node_list->push_back(C.node);
}

block(A) ::= L_BRACE statement_list(B) R_BRACE. {
    A.node = new NodeNode(NODE_BLOCK, B.node);
}
block(A) ::= L_BRACE R_BRACE. {
    A.node = new VoidNode(NODE_VOID);
}

statement_list(A) ::= statement(B). {
    A.node = B.node;
}
statement_list(A) ::= statement_list(B) statement(C).  {
    A.node = new BinaryNode(NODE_STMTS, B.node, C.node);
}

expression(A) ::= assignment_expression(B). {
    A.node = B.node;
}

assignment_expression(A) ::= conditional_expression(B). {
    A.node = B.node;
}
assignment_expression(A) ::= MY variable(B) ASSIGN expression(C).  {
    // TODO: REMOVE THIS NODE
    Node *n1 = new NodeNode(NODE_MY, B.node);
    Node *node = new BinaryNode(NODE_SETVARIABLE, B.node, C.node);
    A.node = new BinaryNode(NODE_STMTS, n1, node);
}
assignment_expression(A) ::= conditional_expression(B) ASSIGN conditional_expression(C).  {
    A.node = new BinaryNode(NODE_SETVARIABLE, B.node, C.node);
}
assignment_expression(A) ::= conditional_expression(B) DIV_ASSIGN conditional_expression(C).  {
    A.node = new BinaryNode(NODE_DIV_ASSIGN, B.node, C.node);
}

conditional_expression(A) ::= logical_or_expression(B). { A.node = B.node; }

logical_or_expression(A) ::= logical_and_expression(B). { A.node = B.node; }

logical_and_expression(A) ::= inclusive_or_expression(B). { A.node = B.node; }

inclusive_or_expression(A) ::= exclusive_or_expression(B). { A.node = B.node; }

exclusive_or_expression(A) ::= and_expression(B). { A.node = B.node; }

and_expression(A) ::= equality_expression(B). { A.node = B.node; }

equality_expression(A) ::= relational_expression(B). { A.node = B.node; }
equality_expression(A) ::= equality_expression(B) EQ relational_expression(C). {
    A.node = new BinaryNode(NODE_EQ, B.node, C.node);
}

relational_expression(A) ::= shift_expression(B). { A.node = B.node; }
relational_expression(A) ::= relational_expression(B) LT shift_expression(C). {
    A.node = new BinaryNode(NODE_LT, B.node, C.node);
}
relational_expression(A) ::= relational_expression(B) GT shift_expression(C). {
    A.node = new BinaryNode(NODE_GT, B.node, C.node);
}
relational_expression(A) ::= relational_expression(B) LE shift_expression(C). {
    A.node = new BinaryNode(NODE_LE, B.node, C.node);
}
relational_expression(A) ::= relational_expression(B) GE shift_expression(C). {
    A.node = new BinaryNode(NODE_GE, B.node, C.node);
}

shift_expression(A) ::= additive_expression(B). { A.node = B.node; }

additive_expression(A) ::= multiplicative_expression(B). { A.node = B.node; }
additive_expression(A) ::= additive_expression(B) ADD multiplicative_expression(C). {
    A.node = new BinaryNode(NODE_ADD, B.node, C.node);
}
additive_expression(A) ::= additive_expression(B) SUB multiplicative_expression(C). {
    A.node = new BinaryNode(NODE_SUB, B.node, C.node);
}

multiplicative_expression(A) ::= unary_expression(B). { A.node = B.node; }
multiplicative_expression(A) ::= multiplicative_expression(B) MUL unary_expression(C). {
    A.node = new BinaryNode(NODE_MUL, B.node, C.node);
}
multiplicative_expression(A) ::= multiplicative_expression(B) DIV unary_expression(C). {
    A.node = new BinaryNode(NODE_DIV, B.node, C.node);
}

unary_expression(A) ::= postfix_expression(B). { A.node = B.node; }
unary_expression(A) ::= /* ++$i */ PLUSPLUS unary_expression(B). {
    A.node = new NodeNode(NODE_UNARY_INCREMENT, B.node);
}
unary_expression(A) ::= SUB unary_expression(B). {
    A.node = new NodeNode(NODE_UNARY_NEGATIVE, B.node);
}

postfix_expression(A) ::= primary_expression(B). { A.node = B.node; }
postfix_expression(A) ::= primary_expression(B) L_BRACKET expression(C) R_BRACKET. {
    A.node = new BinaryNode(NODE_GET_ITEM, B.node, C.node);
}
postfix_expression(A) ::= identifier(B) L_PAREN R_PAREN. {
    A.node = new FuncallNode(B.node, new std::vector<SharedPtr<Node>>());
}
postfix_expression(A) ::= identifier(B) L_PAREN argument_list(C) R_PAREN. {
    // TODO: support vargs
    A.node = new FuncallNode(B.node, C.node_list);
}
postfix_expression(A) ::= primary_expression(B) DOT identifier(C) L_PAREN argument_list(D) R_PAREN.  {
    A.node = new MethodCallNode(B.node, C.node, D.node_list);
}
postfix_expression(A) ::= primary_expression(B) DOT identifier(C) L_PAREN R_PAREN. {
    A.node = new MethodCallNode(B.node, C.node, new std::vector<SharedPtr<Node>>());
}

primary_expression(A) ::= int(B). { A.node = B.node; }
primary_expression(A) ::= DOUBLE_LITERAL(B). {
    A.node = new DoubleNode(NODE_DOUBLE, B.double_value);
}
primary_expression(A) ::= int(B) DOTDOT int(C). {
    A.node = new BinaryNode(NODE_RANGE, B.node, C.node);
}
primary_expression(A) ::= FALSE. {
    A.node = new VoidNode(NODE_FALSE);
}
primary_expression(A) ::= TRUE. {
    A.node = new VoidNode(NODE_TRUE);
}
primary_expression(A) ::= REGEXP_LITERAL(B). {
    A.node = new RegexpNode(NODE_REGEXP, B.str_value);
}
primary_expression(A) ::= STRING_LITERAL(B). {
    A.node = new StrNode(NODE_STRING, B.str_value);
}
primary_expression(A) ::= variable(B). { A.node = B.node; }
primary_expression(A) ::= array_creation(B). { A.node = B.node; }
primary_expression(A) ::= L_PAREN expression(B) R_PAREN. {
    A.node = B.node;
}

int(A) ::= INT_LITERAL(B). {
    A.node = new IntNode(NODE_INT, B.int_value);
}

identifier(A) ::= IDENTIFIER(B). {
    A.node = new StrNode(NODE_IDENTIFIER, B.str_value);
}

variable(A) ::= VARIABLE(B). {
    A.node = new StrNode(NODE_GETVARIABLE, B.str_value);
}

