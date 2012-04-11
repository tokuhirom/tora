/* vim: set filetype=lemon: */
/**
    perlop is following:

           left        or xor
           left        and
           right       not
           nonassoc    list operators (rightward)
           left        , =>
           right       = += -= *= etc.
           right       ?:
           nonassoc    ..  ...
           left        || //
           left        &&
           left        | ^
           left        &
           nonassoc    == != <=> eq ne cmp ~~
           nonassoc    < > <= >= lt gt le ge
           nonassoc    named unary operators
           left        << >>
           left        + - .
           left        * / % x
           left        =~ !~
           right       ! ~ \ and unary + and -
           right       **
           nonassoc    ++ --
           left        ->
           left        terms and list operators (leftward)

Missing part is following:

    *= -= &= |= ^= %=
    not
    and
    or xor

 */

%right FILE_TEST.
%right MY OUR.
%left QW_START QW_END.
%left L_BRACE R_BRACE.
%left L_BRACKET R_BRACKET.

%right LAMBDA.
%left COMMA FAT_COMMA.
%right ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN AND_ASSIGN OR_ASSIGN XOR_ASSIGN MOD_ASSIGN OROR_ASSIGN.
%left DOTDOT.
%left OROR.
%left ANDAND.
%left BITOR BITXOR.
%left BITAND.
%nonassoc EQ.
%nonassoc LT GT LE GE.
%left BITLSHIFT BITRSHIFT.
%left ADD SUB.
%left MUL DIV MOD.
%right POW.
%nonassoc PLUSPLUS MINUSMINUS.
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
    fprintf(stderr, "Syntax error %s at line %d(%s).\n", state->filename.c_str(), state->lineno, token_id2name[yymajor]);
}

%extra_argument { ParserState *state }

%stack_overflow {
    state->errors++;
    fprintf(stderr,"Giving up.  Parser stack overflow?n");
}

%start_symbol root

root ::= translation_unit(A). {
    state->root_node = new Node(NODE_ROOT, A);
}
root ::= translation_unit(A) expression(B). {
    state->root_node = new Node(NODE_ROOT,
        new BinaryNode(NODE_STMTS, A, B));
}
root ::= expression(A). {
    state->root_node = new Node(NODE_ROOT, A);
}
root ::= . {
    state->root_node = new Node(NODE_ROOT, new Node(NODE_VOID));
}

translation_unit(A) ::= statement(B). { A = B; }
translation_unit(A) ::= translation_unit(B) statement(C). {
    A = new BinaryNode(NODE_STMTS, B, C);
}

expression(A) ::= LAST. {
    A = new Node(NODE_LAST);
}
expression(A) ::= NEXT. {
    A = new Node(NODE_NEXT);
}
expression(A) ::= REDO. {
    A = new Node(NODE_REDO);
}

statement(A) ::= expression(B) SEMICOLON . {
    A = B;
}
statement(A) ::= expression(B) UNLESS expression(C) SEMICOLON. {
    A = new IfNode(NODE_IF, new Node(NODE_NOT, C), B, NULL);
}
statement(A) ::= expression(B) IF expression(C) SEMICOLON. {
    A = new IfNode(NODE_IF, C, B, NULL);
}
statement(A) ::= expression(B) WHILE expression(C) SEMICOLON. {
    A = new BinaryNode(NODE_WHILE, C, B);
}
statement(A) ::= expression(B) FOR expression(C) SEMICOLON. {
    A = new ForEachNode(C, NULL, B);
}
statement(A) ::= SEMICOLON. {
    A = new Node(NODE_VOID);
}
statement(A) ::= jump_statement(B). { A = B; }
statement(A) ::= if_statement(B).   { A = B; }
statement(A) ::= WHILE expression(B) maybe_block(C). {
    A = new BinaryNode(NODE_WHILE, B, C);
}
statement(A) ::= FOR L_PAREN expression(B) SEMICOLON expression(C) SEMICOLON expression(D) R_PAREN maybe_block(E). {
    A = new ForNode(B, C, D, E);
}
statement(A) ::= FOR expression(B) LAMBDA lambda_parameter_list(C) maybe_block(D). {
    A = new ForEachNode(B, C, D);
}
statement(A) ::= FOR expression(B) LAMBDA maybe_block(C). {
    A = new ForEachNode(B, NULL, C);
}
statement(A) ::= sub_stmt(B).   { A = B; }
statement(A) ::= block(B).   { A = B; }
statement(A) ::= class_statement(B). { A = B; }

class_statement(A) ::= CLASS identifier(B) maybe_block(C). {
    A = new ClassNode(B, NULL, NULL, C);
}
class_statement(A) ::= CLASS identifier(B) IS identifier(C) maybe_block(D). {
    A = new ClassNode(B, C, NULL, D);
}

jump_statement(A) ::= RETURN argument_list(B) SEMICOLON. {
    B->type = NODE_RETURN;
    A = B;
}

if_statement(A) ::= UNLESS expression(B) maybe_block(C). {
    A = new IfNode(NODE_IF, new Node(NODE_NOT, B), C, NULL);
}

if_statement(A) ::= IF expression(B) maybe_block(C). {
    A = new IfNode(NODE_IF, B, C, NULL);
}
if_statement(A) ::= IF expression(B) maybe_block(C) elsif_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
if_statement(A) ::= IF expression(B) maybe_block(C) else_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
elsif_clause(A) ::= ELSIF expression(B) maybe_block(C). {
    A = new IfNode(NODE_IF, B, C, NULL);
}
elsif_clause(A) ::= ELSIF expression(B) maybe_block(C) else_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
elsif_clause(A) ::= ELSIF expression(B) maybe_block(C) elsif_clause(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
else_clause(A) ::= ELSE maybe_block(B). {
    A = B;
}

sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN sub_parameter_list(C) R_PAREN maybe_block(D). {
    A = new FuncdefNode(B, C->upcast<Node>(), D);
}
sub_stmt(A) ::= FUNCSUB identifier(B) L_PAREN R_PAREN maybe_block(C). {
    /* sub foo() { } */
    A = new FuncdefNode(B, new Node(), C);
}
sub_stmt(A) ::= FUNCSUB identifier(B) maybe_block(C). {
    A = new FuncdefNode(B, NULL, C);
}

lambda_parameter_list(A) ::= variable(B). {
    A = new Node();
    A->upcast<Node>()->push_back(B);
}
lambda_parameter_list(A) ::= lambda_parameter_list(B) COMMA variable(C). {
    A = B;
    A->upcast<Node>()->push_back(C);
}

/* the arguments in function/lambda definition. */
sub_parameter_list(A) ::= parameter(B). {
    A = new Node();
    A->upcast<Node>()->push_back(B);
}
sub_parameter_list(A) ::= sub_parameter_list(B) COMMA parameter(C). {
    A = B;
    A->upcast<Node>()->push_back(C);
}
parameter(A) ::= variable(B). {
    Node *n = new Node();
    n->push_back(B);
    n->push_back(NULL);
    A = n;
}
parameter(A) ::= variable(B) ASSIGN expression(C). {
    Node *n = new Node();
    n->push_back(B);
    n->push_back(C);
    A = n;
}

/* allow [a,b,c,] */

/* array constructor [a, b] */
array_creation(A) ::= L_BRACKET argument_list(B) R_BRACKET. {
    B->type = NODE_MAKE_ARRAY;
    A = B;
}
array_creation(A) ::= L_BRACKET R_BRACKET. {
    A = new Node(NODE_MAKE_ARRAY);
}

hash_creation(A) ::= L_BRACE pair_list(B) COMMA R_BRACE. {
    B->type = NODE_MAKE_HASH;
    A = B;
}
hash_creation(A) ::= L_BRACE pair_list(B) R_BRACE. {
    B->type = NODE_MAKE_HASH;
    A = B;
}
hash_creation(A) ::= L_BRACE R_BRACE. {
    A = new Node(NODE_MAKE_HASH);
}

argument_list(A) ::= expression(B). {
    A = new Node();
    A->upcast<Node>()->push_back(B);
}
argument_list(A) ::= argument_list(B) COMMA expression(C). {
    A = B;
    A->upcast<Node>()->push_back(C);
}

/* B => C */
pair_list(A) ::= expression(B) FAT_COMMA expression(C). {
    A = new Node();
    A->upcast<Node>()->push_back(B);
    A->upcast<Node>()->push_back(C);
}
pair_list(A) ::= pair_list(B) COMMA expression(C) FAT_COMMA expression(D). {
    A = B;
    A->upcast<Node>()->push_back(C);
    A->upcast<Node>()->push_back(D);
}

maybe_block(A) ::= block(B). {
    A = B;
}
maybe_block(A) ::= L_BRACE R_BRACE. {
    A = new Node(NODE_VOID);
}
block(A) ::= L_BRACE statement_list(B) R_BRACE. {
    A = new Node(NODE_BLOCK, B);
}
block(A) ::= L_BRACE expression(B) R_BRACE. {
    Node* ln = new Node(NODE_STMTS_LIST);
    ln->push_back(B);
    A = new Node(NODE_BLOCK, ln);
}

statement_list(A) ::= statement_list_inner(B).  {
    A = B;
}
statement_list(A) ::= statement_list_inner(B) expression(C).  {
    B->upcast<Node>()->push_back(C);
    A = B;
}
statement_list_inner(A) ::= statement(B). {
    Node* ln = new Node(NODE_STMTS_LIST);
    ln->push_back(B);
    A = ln;
}
statement_list_inner(A) ::= statement_list_inner(B) statement(C). {
    B->upcast<Node>()->push_back(C);
    A = B;
}

expression(A) ::= assignment_expression(B). {
    A = B;
}
expression(A) ::= LOCAL variable(B).   {
    Node * ln = new Node(NODE_LOCAL);
    ln->push_back(B);
    ln->push_back(NULL);
    A = ln;
}
expression(A) ::= LOCAL variable(B) ASSIGN expression(C).   {
    Node * ln = new Node(NODE_LOCAL);
    ln->push_back(B);
    ln->push_back(C);
    A = ln;
}
expression(A) ::= DIE expression(B). {
    A = new Node(NODE_DIE, B);
}
expression(A) ::= use_expression(B).   { A = B; }
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
assignment_expression(A) ::= conditional_expression(B) ADD_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_ADD_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) SUB_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_SUB_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) DIV_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_DIV_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) MUL_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_MUL_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) AND_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_AND_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) OR_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_OR_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) OROR_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_OROR_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) XOR_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_XOR_ASSIGN, B, C);
}
assignment_expression(A) ::= conditional_expression(B) MOD_ASSIGN conditional_expression(C).  {
    A = new BinaryNode(NODE_MOD_ASSIGN, B, C);
}

conditional_expression(A) ::= logical_or_expression(B). { A = B; }
conditional_expression(A) ::= logical_or_expression(B) QUESTION expression(C) COLON conditional_expression(D). {
    A = new IfNode(NODE_IF, B, C, D);
}
conditional_expression(A) ::= MY conditional_expression(B).   {
    Node*nl = new Node(NODE_MY);
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
inclusive_or_expression(A) ::= inclusive_or_expression(B) BITOR exclusive_or_expression(C). {
    A = new BinaryNode(NODE_BITOR, B, C);
}

exclusive_or_expression(A) ::= and_expression(B). { A = B; }
exclusive_or_expression(A) ::= exclusive_or_expression(B) BITXOR and_expression(C). {
    A = new BinaryNode(NODE_BITXOR, B, C);
}

and_expression(A) ::= equality_expression(B). { A = B; }
and_expression(A) ::= and_expression(B) BITAND equality_expression(C). {
    A = new BinaryNode(NODE_BITAND, B, C);
}

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
shift_expression(A) ::= shift_expression(B) BITLSHIFT additive_expression(C). {
    A = new BinaryNode(NODE_BITLSHIFT, B, C);
}
shift_expression(A) ::= shift_expression(B) BITRSHIFT additive_expression(C). {
    A = new BinaryNode(NODE_BITRSHIFT, B, C);
}

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
multiplicative_expression(A) ::= multiplicative_expression(B) MOD unary_expression(C). {
    A = new BinaryNode(NODE_MOD, B, C);
}
multiplicative_expression(A) ::= multiplicative_expression(B) DIV unary_expression(C). {
    A = new BinaryNode(NODE_DIV, B, C);
}
multiplicative_expression(A) ::= multiplicative_expression(B) POW unary_expression(C). {
    A = new BinaryNode(NODE_POW, B, C);
}

unary_expression(A) ::= postfix_expression(B). { A = B; }
unary_expression(A) ::= /* --$i */ MINUSMINUS unary_expression(B). {
    A = new Node(NODE_PRE_DECREMENT, B);
}
unary_expression(A) ::= /* ++$i */ PLUSPLUS unary_expression(B). {
    A = new Node(NODE_PRE_INCREMENT, B);
}
unary_expression(A) ::= /* -f $file */ FILE_TEST(B) unary_expression(C). {
    A = new BinaryNode(NODE_FILE_TEST, B, C);
}
unary_expression(A) ::= NOT unary_expression(B). {
    A = new Node(NODE_NOT, B);
}
unary_expression(A) ::= SUB unary_expression(B). {
    A = new Node(NODE_UNARY_NEGATIVE, B);
}
/* my ($err, $ret) = try { }; */
unary_expression(A) ::= TRY maybe_block(B). {
    A = new Node(NODE_TRY, B);
}
unary_expression(A) ::= LAMBDA lambda_parameter_list(B) maybe_block(C). {
    A = new FuncdefNode(NULL, B->upcast<Node>(), C);
    A->type = NODE_LAMBDA;
}
unary_expression(A) ::= LAMBDA maybe_block(C). {
    A = new FuncdefNode(NULL, new Node(), C);
    A->type = NODE_LAMBDA;
}
unary_expression(A) ::= MUL unary_expression(C). {
    A = new Node(NODE_EXTRACT_ARRAY, C);
}
unary_expression(A) ::= identifier(B). {
    B->type = NODE_INSTANCIATE_IDENTIFIER;
    A = B;
}

postfix_expression(A) ::= primary_expression(B). { A = B; }
postfix_expression(A) ::= primary_expression(B) L_BRACKET expression(C) R_BRACKET. {
    A = new BinaryNode(NODE_GET_ITEM, B, C);
}
postfix_expression(A) ::= identifier(B) L_PAREN R_PAREN. {
    A = new FuncallNode(B, new Node());
}
postfix_expression(A) ::= identifier(B) L_PAREN argument_list(C) R_PAREN. {
    // TODO: support vargs
    A = new FuncallNode(B, C->upcast<Node>());
}
postfix_expression(A) ::= identifier(B) DOT identifier(C) L_PAREN argument_list(D) R_PAREN.  {
    B->type = NODE_INSTANCIATE_IDENTIFIER;
    A = new MethodCallNode(B, C, D->upcast<Node>());
}
postfix_expression(A) ::= identifier(B) DOT identifier(C) L_PAREN R_PAREN.  {
    B->type = NODE_INSTANCIATE_IDENTIFIER;
    A = new MethodCallNode(B, C, new Node());
}
postfix_expression(A) ::= identifier(B) DOT identifier(C).  {
    B->type = NODE_INSTANCIATE_IDENTIFIER;
    // Foo.bar
    A = new MethodCallNode(B, C, new Node());
}
postfix_expression(A) ::= postfix_expression(B) DOT L_PAREN R_PAREN.  {
    // $foo.();
    A = new MethodCallNode(B, NULL, new Node());
}
postfix_expression(A) ::= postfix_expression(B) DOT L_PAREN argument_list(C) R_PAREN.  {
    // $foo.(1,2,3);
    A = new MethodCallNode(B, NULL, C->upcast<Node>());
}
postfix_expression(A) ::= postfix_expression(B) DOT identifier(C) L_PAREN argument_list(D) R_PAREN.  {
    A = new MethodCallNode(B, C, D->upcast<Node>());
}
postfix_expression(A) ::= postfix_expression(B) DOT identifier(C) L_PAREN R_PAREN. {
    A = new MethodCallNode(B, C, new Node());
}
postfix_expression(A) ::= postfix_expression(B) DOT identifier(C). {
    // $foo.bar
    A = new MethodCallNode(B, C, new Node());
}
postfix_expression(A) ::= /* $i-- */ postfix_expression(B) MINUSMINUS. {
    A = new Node(NODE_POST_DECREMENT, B);
}
postfix_expression(A) ::= /* $i++ */ postfix_expression(B) PLUSPLUS. {
    A = new Node(NODE_POST_INCREMENT, B);
}

primary_expression(A) ::= DEREF expression(B) R_BRACE. {
    /* ${ obj } */
    A = new Node(NODE_DEREF, B);
}
primary_expression(A) ::= int(B). { A = B; }
primary_expression(A) ::= DOUBLE_LITERAL(B). {
    A = B;
}
primary_expression(A) ::= primary_expression(B) DOTDOT primary_expression(C). {
    A = new BinaryNode(NODE_RANGE, B, C);
}
primary_expression(A) ::= FALSE. {
    A = new Node(NODE_FALSE);
}
primary_expression(A) ::= SELF. {
    A = new Node(NODE_SELF);
}
primary_expression(A) ::= TRUE. {
    A = new Node(NODE_TRUE);
}
primary_expression(A) ::= REGEXP_LITERAL(B). {
    B->type = NODE_REGEXP;
    A = B;
}
primary_expression(A) ::= STRING_LITERAL(B). {
    B->type = NODE_STRING;
    A = B;
}
primary_expression(A) ::= BYTES_LITERAL(B). {
    B->type = NODE_BYTES;
    A = B;
}
primary_expression(A) ::= variable(B). { A = B; }
primary_expression(A) ::= array_creation(B). { A = B; }
primary_expression(A) ::= qw_creation(B). { A = B; }
primary_expression(A) ::= hash_creation(B). { A = B; }
/* tuple */
primary_expression(A) ::= L_PAREN tuple_list(B) R_PAREN. {
    A = B;
}
primary_expression(A) ::= L_PAREN expression(B) R_PAREN. {
    A = B;
}
tuple_list(A) ::= expression(B) COMMA expression(C). {
    Node * n = new Node(NODE_TUPLE);
    n->push_back(B);
    n->push_back(C);
    A = n;
}
tuple_list(A) ::= tuple_list(B) COMMA expression(C). {
    A = B;
    A->upcast<Node>()->push_back(C);
}
primary_expression(A) ::= UNDEF. {
    A = new Node(NODE_UNDEF);
}
primary_expression(A) ::= DOTDOTDOT. {
    A = new Node(NODE_DOTDOTDOT);
}
primary_expression(A) ::= HEREDOC_START(B). {
    A = B;
}
primary_expression(A) ::= PACKAGE_LITERAL. {
    A = new FuncallNode(new StrNode(NODE_IDENTIFIER, "__PACKAGE__"), new Node());
}
primary_expression(A) ::= FUNCSUB L_PAREN R_PAREN maybe_block(B). {
    A = new FuncdefNode(new StrNode(NODE_IDENTIFIER, "<anonymous>"), new Node(), B);
}

/* qw */
qw_creation(A) ::= QW_START qw_list(B) QW_END. {
    B->type = NODE_MAKE_ARRAY;
    A = B;
}
qw_creation(A) ::= QW_START QW_END. {
    A = new Node(NODE_MAKE_ARRAY);
}
qw_list(A) ::= QW_WORD(B). {
    Node *n = new Node();
    n->push_back(B);
    A = n;
}
qw_list(A) ::= qw_list(B) QW_WORD(C). {
    B->upcast<Node>()->push_back(C);
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
identifier(A) ::= IS(B). {
    B->type = NODE_IDENTIFIER;
    A = B;
}

variable(A) ::= VARIABLE(B). {
    B->type = NODE_GETVARIABLE;
    A = B;
}

