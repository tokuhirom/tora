#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use autodie;
use 5.10.0;

my @ops = qw(
    OP_NOP
    OP_ADD
    OP_SUB
    OP_DIV
    OP_MUL
    OP_PRINT
    OP_PUSH_INT
    OP_PUSH_DOUBLE
    OP_PUSH_TRUE
    OP_PUSH_FALSE
    OP_PUSH_IDENTIFIER
    OP_NEW_RANGE
    OP_FUNCALL
    OP_PUSH_STRING
    OP_PUSH_VALUE
    OP_DUMP
    OP_JUMP_IF_FALSE

    OP_LT
    OP_GT
    OP_LE
    OP_GE
    OP_EQ

    OP_ASSIGN
    OP_JUMP
    OP_END
    OP_RETURN
    OP_ENTER
    OP_LEAVE

    OP_SETLOCAL
    OP_SETDYNAMIC
    OP_GETGLOBAL
    OP_GETLOCAL
    OP_GETDYNAMIC

    OP_DEFINE_METHOD

    OP_MAKE_ARRAY

    OP_GET_ITEM
    OP_SET_ITEM

    OP_UNARY_NEGATIVE

    OP_METHOD_CALL

    OP_UNARY_INCREMENT
);

printf <<'...', join(", ", @ops);
#ifndef OPS_GEN_H_
#define OPS_GEN_H_

namespace tora {

typedef enum {%s} op_type_t;

extern const char*opcode2name[];

};

#endif // OPS_GEN_H_
...

open my $fh, '>', 'ops.gen.cc';
printf $fh (<<'...', join(", ", map{ qq{"$_"} } @ops));
#include "ops.gen.h"

const char*tora::opcode2name[] = {%s};
...

