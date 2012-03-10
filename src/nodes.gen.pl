#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;

my @nodes = qw(
    NODE_INT
    NODE_DOUBLE
    NODE_TRUE
    NODE_FALSE
    NODE_UNDEF
    NODE_IDENTIFIER
    NODE_TUPLE
    NODE_FUNCALL
    NODE_STRING
    NODE_ADD
    NODE_SUB
    NODE_MUL
    NODE_MOD
    NODE_DIV
    NODE_POW
    NODE_STMTS
    NODE_STMTS_LIST
    NODE_NEWLINE
    NODE_IF

    NODE_DEREF
    NODE_SELF

    NODE_LAMBDA

    NODE_LT
    NODE_GT
    NODE_LE
    NODE_GE
    NODE_EQ
    NODE_NE
    NODE_BITXOR
    NODE_BITOR
    NODE_BITAND
    NODE_BITLSHIFT
    NODE_BITRSHIFT
    NODE_GETVARIABLE
    NODE_SETVARIABLE
    NODE_SETVARIABLE_MULTI
    NODE_ASSIGN
    NODE_WHILE
    NODE_ROOT
    NODE_VOID
    NODE_FUNCDEF
    NODE_RETURN
    NODE_BLOCK
    NODE_MY
    NODE_MAKE_ARRAY
    NODE_MAKE_HASH
    NODE_GET_ITEM
    NODE_FOR
    NODE_FOREACH
    NODE_UNARY_NEGATIVE
    NODE_METHOD_CALL

    NODE_PRE_INCREMENT
    NODE_POST_INCREMENT
    NODE_PRE_DECREMENT
    NODE_POST_DECREMENT

    NODE_NOT

    NODE_FILE_TEST

    NODE_RANGE
    NODE_REGEXP
    NODE_DOTDOTDOT

    NODE_TRY
    NODE_DIE

    NODE_USE

    NODE_CLASS

    NODE_LOGICAL_AND
    NODE_LOGICAL_OR

    NODE_REDO
    NODE_LAST
    NODE_NEXT

    NODE_ADD_ASSIGN
    NODE_SUB_ASSIGN
    NODE_DIV_ASSIGN
    NODE_MUL_ASSIGN
    NODE_AND_ASSIGN
    NODE_OR_ASSIGN
    NODE_XOR_ASSIGN
    NODE_MOD_ASSIGN
);

open my $cc, '>', 'src/nodes.gen.cc';

printf (<<'...', join(", ", @nodes));
#ifndef NODES_GEN_H_
#define NODES_GEN_H_

namespace tora {

typedef enum {
    NODE_UNKNOWN = 0,
%s} node_type_t;

extern const char*node_type2name[];

};

#endif // NODES_GEN_H_
...


printf $cc (<<'...', join(", ", map { qq{"$_"} } @nodes) );
#include "nodes.gen.h"

const char*tora::node_type2name[] = {"NODE_UNKNOWN", %s};
...


