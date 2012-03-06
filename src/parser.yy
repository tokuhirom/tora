/* vim: set filetype=lemon: */
/**
    perlop is following:

           left        terms and list operators (leftward)
           left        ->
           nonassoc    ++ --
           right       **
           right       ! ~ \ and unary + and -
           left        =~ !~
           left        * / % x
           left        + - .
           left        << >>
           nonassoc    named unary operators
           nonassoc    < > <= >= lt gt le ge
           nonassoc    == != <=> eq ne cmp ~~
           left        &
           left        | ^
           left        &&
           left        || //
           nonassoc    ..  ...
           right       ?:
           right       = += -= *= etc.
           left        , =>
           nonassoc    list operators (rightward)
           right       not
           left        and
           left        or xor

Missing part is following:

    **
    %
    & | ^ 
    *= -= &= |= ^= %=
    not
    and
    or xor

 */

%right FILE_TEST.
%right MY.
%left QW_START QW_END.
%left L_BRACE R_BRACE.
%left L_BRACKET R_BRACKET.

%right ASSIGN DIV_ASSIGN.
%left DOTDOT.
%left OROR.
%left ANDAND.
%nonassoc EQ.
%nonassoc LT GT LE GE.
%left ADD SUB.
%left MUL DIV.
%left PLUSPLUS MINUSMINUS.
%right NOT.
%left DOT.

%token_type { Node* }

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
#include "shared_ptr.h"
#include "value/regexp.h"
#include "parser.h"
#include "lexer.h"
#include "token.gen.h"

using namespace tora;

// see http://www.lysator.liu.se/c/ANSI-C-grammar-y.html

}

%parse_failure {
    state->failure = true;
    fprintf(stderr, "Parse aborted at line %d.\n", state->lineno);
}
%syntax_error {
    state->errors++;
    fprintf(stderr, "Syntax error at line %d(%s).\n", state->lineno, token_id2name[yymajor]);
}

%extra_argument { ParserState *state }

%stack_overflow {
    state->errors++;
    fprintf(stderr,"Giving up.  Parser stack overflow?n");
}

%start_symbol root

root ::= translation_unit(A). {
    state->root_node = new NodeNode(NODE_ROOT, A);
}
root ::= . {
    state->root_node = new NodeNode(NODE_ROOT, new VoidNode(NODE_VOID));
}

translation_unit(A) ::= statement(B). { A = B; }
translation_unit(A) ::= translation_unit(B) statement(C). {
    A = new BinaryNode(NODE_STMTS, B, C);
}

expression(A) ::= LAST. {
    A = new Node(NODE_LAST);
}
expression(A) ::= NEXT. {
    A = new VoidNode(NODE_NEXT);
}
expression(A) ::= REDO. {
    A = new VoidNode(NODE_REDO);
}

statement(A) ::= expression(B) SEMICOLON . {
    A = B;
}
statement(A) ::= expression(B) IF expression(C) SEMICOLON. {
    A = new IfNode(NODE_IF, C, B, NULL);
}
statement(A) ::= SEMICOLON. {
    A = new VoidNode(NODE_VOID);
}
statement(A) ::= jump_statement(B). { A = B; }
statement(A) ::= if_statement(B).   { A = B; }
statement(A) ::= WHILE L_PAREN expression(B) R_PAREN block(C). {
    A = new BinaryNode(NODE_WHILE, B, C);
}
statement(A) ::= WHILE L_PAREN expression(B) R_PAREN L_BRACE R_BRACE. {
    A = new BinaryNode(NODE_WHILE, B, new ListNode(NODE_STMTS_LIST));
}
statement(A) ::= FOR L_PAREN expression(B) SEMICOLON expression(C) SEMICOLON expression(D) R_PAREN block(E). {
    A = new ForNode(B, C, D, E);
}
statement(A) ::= FOR L_PAREN expression(B) SEMICOLON expression(C) SEMICOLON expression(D) R_PAREN L_BRACE R_BRACE. {
    A = new ForNode(B, C, D, new VoidNode(NODE_VOID));
}
statement(A) ::= FOR L_PAREN expression(B) IN expression(C) R_PAREN block(D). {
    A = new ForEachNode(B, C, D);
}
statement(A) ::= FOR L_PAREN expression(B) IN expression(C) R_PAREN L_BRACE R_BRACE. {
    A = new ForEachNode(B, C, new VoidNode(NODE_VOID));
}
statement(A) ::= sub_stmt(B).   { A = B; }
statement(A) ::= block(B).   { A = B; }
statement(A) ::= class_statement(B). { A = B; }

class_statement(A) ::= CLASS identifier(B) block(C). {
    A = new ClassNode(B, NULL, NULL, C);
}
class_statement(A) ::= CLASS identifier(B) L_BRACE R_BRACE. {
    A = new ClassNode(B, NULL, NULL, NULL);
}

jump_statement(A) ::= RETURN argument_list(B) SEMICOLON. {
    B->type = NODE_RETURN;
    A = B;
}

if_statement(A) ::= IF L_PAREN expression(B) R_PAREN block(C). {
    A = new IfNode(NODE_IF, B, C, NULL);
}
if_statement(A) ::= IF L_PAREN expression(B) R_PAREN L_BRACE R_BRACE. {
    A = new IfNode(NODE_IF, B, NULL, NULL);
}
if_statement(A) ::= IF L_PAREN expression(B) R_PAREN block(C) elsif_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
if_statement(A) ::= IF L_PAREN expression(B) R_PAREN block(C) else_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
elsif_clause(A) ::= ELSIF L_PAREN expression(B) R_PAREN block(C). {
    A = new IfNode(NODE_IF, B, C, NULL);
}
elsif_clause(A) ::= ELSIF L_PAREN expression(B) R_PAREN block(C) else_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
elsif_clause(A) ::= ELSIF L_PAREN expression(B) R_PAREN block(C) elsif_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
else_clause(A) ::= ELSE block(B). {
    A = B;
}

sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN parameter_list(C) R_PAREN block(D). {
    A = new FuncdefNode(B, C->upcast<ListNode>(), D);
}
sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN parameter_list(C) R_PAREN L_BRACE R_BRACE. {
    A = new FuncdefNode(B, C->upcast<ListNode>(), new VoidNode(NODE_VOID));
}
sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN R_PAREN block(C). {
    A = new FuncdefNode(B, new ListNode(), C);
}
sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN R_PAREN L_BRACE R_BRACE. {
    A = new FuncdefNode(B, new ListNode(), new VoidNode(NODE_VOID));
}

parameter_list(A) ::= expression(B). {
    A = new ListNode();
    A->upcast<ListNode>()->push_back(B);
}
parameter_list(A) ::= parameter_list(B) COMMA expression(C). {
    A = B;
    A->upcast<ListNode>()->push_back(C);
}
/* allow [a,b,c,] */

/* array constructor [a, b] */
array_creation(A) ::= L_BRACKET argument_list(B) R_BRACKET. {
    B->type = NODE_MAKE_ARRAY;
    A = B;
}
array_creation(A) ::= L_BRACKET R_BRACKET. {
    A = new ListNode(NODE_MAKE_ARRAY);
}

hash_creation(A) ::= L_BRACE pair_list(B) R_BRACE. {
    B->type = NODE_MAKE_HASH;
    A = B;
}
hash_creation(A) ::= L_BRACE R_BRACE. {
    A = new ListNode(NODE_MAKE_HASH);
}

argument_list(A) ::= expression(B). {
    A = new ListNode();
    A->upcast<ListNode>()->push_back(B);
}
argument_list(A) ::= argument_list(B) COMMA expression(C). {
    A = B;
    A->upcast<ListNode>()->push_back(C);
}

/* B => C */
pair_list(A) ::= expression(B) FAT_COMMA expression(C). {
    A = new ListNode();
    A->upcast<ListNode>()->push_back(B);
    A->upcast<ListNode>()->push_back(C);
}
pair_list(A) ::= pair_list(B) COMMA expression(C) FAT_COMMA expression(D). {
    A = B;
    A->upcast<ListNode>()->push_back(C);
    A->upcast<ListNode>()->push_back(D);
}

block(A) ::= L_BRACE statement_list(B) R_BRACE. {
    A = new NodeNode(NODE_BLOCK, B);
}
block(A) ::= L_BRACE expression(B) R_BRACE. {
    ListNode* ln = new ListNode(NODE_STMTS_LIST);
    ln->push_back(B);
    A = new NodeNode(NODE_BLOCK, ln);
}

statement_list(A) ::= statement(B). {
    ListNode* ln = new ListNode(NODE_STMTS_LIST);
    ln->push_back(B);
    A = ln;
}
statement_list(A) ::= statement_list(B) statement(C).  {
    B->upcast<ListNode>()->push_back(C);
    A = B;
}

expression(A) ::= assignment_expression(B). {
    A = B;
}
expression(A) ::= DIE expression(B). {
    A = new NodeNode(NODE_DIE, B);
}
expression(A) ::= use_expression(B).   { A = B; }
expression(A) ::= identifier(B). {
    A = new FuncallNode(B, new ListNode());
}
use_expression(A) ::= USE identifier(B) MUL. {
    A = new BinaryNode(NODE_USE, B, new IntNode(NODE_INT, 1));
}
use_expression(A) ::= USE identifier(B). {
    A = new BinaryNode(NODE_USE, B, new IntNode(NODE_INT, 0));
}

assignment_expression(A) ::= conditional_expression(B). {
    A = B;
}
assignment_expression(A) ::= conditional_expression(B) ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_SETVARIABLE, B, C);
}
assignment_expression(A) ::= conditional_expression(B) DIV_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_DIV_ASSIGN, B, C);
}

conditional_expression(A) ::= logical_or_expression(B). { A = B; }
conditional_expression(A) ::= logical_or_expression(B) QUESTION expression(C) COLON conditional_expression(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
conditional_expression(A) ::= MY conditional_expression(B).   {
    ListNode*nl = new ListNode(NODE_MY);
    nl->push_back(B);
    A = nl;
}

logical_or_expression(A) ::= logical_and_expression(B). { A = B; }
logical_or_expression(A) ::= logical_or_expression(B) OROR logical_and_expression(C). {
    A = new BinaryNode(NODE_LOGICAL_OR, B, C);
}

/* && */
logical_and_expression(A) ::= inclusive_or_expression(B). { A = B; }
logical_and_expression(A) ::= logical_and_expression(B) ANDAND inclusive_or_expression(C). {
    A = new BinaryNode(NODE_LOGICAL_AND, B, C);
}

inclusive_or_expression(A) ::= exclusive_or_expression(B). { A = B; }

exclusive_or_expression(A) ::= and_expression(B). { A = B; }

and_expression(A) ::= equality_expression(B). { A = B; }

equality_expression(A) ::= relational_expression(B). { A = B; }
equality_expression(A) ::= equality_expression(B) EQ relational_expression(C). {
    A = new BinaryNode(NODE_EQ, B, C);
}
equality_expression(A) ::= equality_expression(B) NE relational_expression(C). {
    A = new BinaryNode(NODE_NE, B, C);
}

relational_expression(A) ::= shift_expression(B). { A = B; }
relational_expression(A) ::= relational_expression(B) LT shift_expression(C). {
    A = new BinaryNode(NODE_LT, B, C);
}
relational_expression(A) ::= relational_expression(B) GT shift_expression(C). {
    A = new BinaryNode(NODE_GT, B, C);
}
relational_expression(A) ::= relational_expression(B) LE shift_expression(C). {
    A = new BinaryNode(NODE_LE, B, C);
}
relational_expression(A) ::= relational_expression(B) GE shift_expression(C). {
    A = new BinaryNode(NODE_GE, B, C);
}

shift_expression(A) ::= additive_expression(B). { A = B; }

additive_expression(A) ::= multiplicative_expression(B). { A = B; }
additive_expression(A) ::= additive_expression(B) ADD multiplicative_expression(C). {
    A = new BinaryNode(NODE_ADD, B, C);
}
additive_expression(A) ::= additive_expression(B) SUB multiplicative_expression(C). {
    A = new BinaryNode(NODE_SUB, B, C);
}

multiplicative_expression(A) ::= unary_expression(B). { A = B; }
multiplicative_expression(A) ::= multiplicative_expression(B) MUL unary_expression(C). {
    A = new BinaryNode(NODE_MUL, B, C);
}
multiplicative_expression(A) ::= multiplicative_expression(B) DIV unary_expression(C). {
    A = new BinaryNode(NODE_DIV, B, C);
}

unary_expression(A) ::= postfix_expression(B). { A = B; }
unary_expression(A) ::= /* --$i */ MINUSMINUS unary_expression(B). {
    A = new NodeNode(NODE_PRE_DECREMENT, B);
}
unary_expression(A) ::= /* ++$i */ PLUSPLUS unary_expression(B). {
    A = new NodeNode(NODE_PRE_INCREMENT, B);
}
unary_expression(A) ::= /* -f $file */ FILE_TEST(B) unary_expression(C). {
    A = new BinaryNode(NODE_FILE_TEST, B, C);
}
unary_expression(A) ::= NOT unary_expression(B). {
    A = new NodeNode(NODE_NOT, B);
}
unary_expression(A) ::= SUB unary_expression(B). {
    A = new NodeNode(NODE_UNARY_NEGATIVE, B);
}
/* my ($err, $ret) = try { }; */
unary_expression(A) ::= TRY block(B). {
    A = new NodeNode(NODE_TRY, B);
}

postfix_expression(A) ::= primary_expression(B). { A = B; }
postfix_expression(A) ::= primary_expression(B) L_BRACKET expression(C) R_BRACKET. {
    A = new BinaryNode(NODE_GET_ITEM, B, C);
}
postfix_expression(A) ::= identifier(B) L_PAREN R_PAREN. {
    A = new FuncallNode(B, new ListNode());
}
postfix_expression(A) ::= identifier(B) L_PAREN argument_list(C) R_PAREN. {
    // TODO: support vargs
    A = new FuncallNode(B, C->upcast<ListNode>());
}
postfix_expression(A) ::= identifier(B) DOT identifier(C) L_PAREN argument_list(D) R_PAREN.  {
    A = new MethodCallNode(B, C, D->upcast<ListNode>());
}
postfix_expression(A) ::= identifier(B) DOT identifier(C) L_PAREN R_PAREN.  {
    A = new MethodCallNode(B, C, new ListNode());
}
postfix_expression(A) ::= postfix_expression(B) DOT identifier(C) L_PAREN argument_list(D) R_PAREN.  {
    A = new MethodCallNode(B, C, D->upcast<ListNode>());
}
postfix_expression(A) ::= postfix_expression(B) DOT identifier(C) L_PAREN R_PAREN. {
    A = new MethodCallNode(B, C, new ListNode());
}
postfix_expression(A) ::= /* $i-- */ postfix_expression(B) MINUSMINUS. {
    A = new NodeNode(NODE_POST_DECREMENT, B);
}
postfix_expression(A) ::= /* $i++ */ postfix_expression(B) PLUSPLUS. {
    A = new NodeNode(NODE_POST_INCREMENT, B);
}

primary_expression(A) ::= int(B). { A = B; }
primary_expression(A) ::= DOUBLE_LITERAL(B). {
    A = B;
}
primary_expression(A) ::= int(B) DOTDOT int(C). {
    A = new BinaryNode(NODE_RANGE, B, C);
}
primary_expression(A) ::= FALSE. {
    A = new VoidNode(NODE_FALSE);
}
primary_expression(A) ::= TRUE. {
    A = new VoidNode(NODE_TRUE);
}
primary_expression(A) ::= REGEXP_LITERAL(B). {
    B->type = NODE_REGEXP;
    A = B;
}
primary_expression(A) ::= STRING_LITERAL(B). {
    B->type = NODE_STRING;
    A = B;
}
primary_expression(A) ::= variable(B). { A = B; }
primary_expression(A) ::= array_creation(B). { A = B; }
primary_expression(A) ::= qw_creation(B). { A = B; }
primary_expression(A) ::= hash_creation(B). { A = B; }
/* tuple */
primary_expression(A) ::= L_PAREN parameter_list(B) R_PAREN. {
    if (B->size() > 1) {
        B->type = NODE_TUPLE;
        A = B;
    } else {
        A = B->at(0);
    }
}
primary_expression(A) ::= UNDEF. {
    A = new VoidNode(NODE_UNDEF);
}
primary_expression(A) ::= DOTDOTDOT. {
    A = new VoidNode(NODE_DOTDOTDOT);
}
primary_expression(A) ::= HEREDOC_START(B). {
    A = B;
}
primary_expression(A) ::= PACKAGE_LITERAL. {
    A = new FuncallNode(new StrNode(NODE_IDENTIFIER, "__PACKAGE__"), new ListNode());
}

/* qw */
qw_creation(A) ::= QW_START qw_list(B) QW_END. {
    B->type = NODE_MAKE_ARRAY;
    A = B;
}
qw_creation(A) ::= QW_START QW_END. {
    A = new ListNode(NODE_MAKE_ARRAY);
}
qw_list(A) ::= QW_WORD(B). {
    ListNode *n = new ListNode();
    n->push_back(B);
    A = n;
}
qw_list(A) ::= qw_list(B) QW_WORD(C). {
    B->upcast<ListNode>()->push_back(C);
    A = B;
}

int(A) ::= INT_LITERAL(B). {
    B->type = NODE_INT;
    A = B;
}

identifier(A) ::= IDENTIFIER(B). {
    B->type = NODE_IDENTIFIER;
    A = B;
}

variable(A) ::= VARIABLE(B). {
    B->type = NODE_GETVARIABLE;
    A = B;
}

