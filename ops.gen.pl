#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use autodie;
use Text::Xslate;
use 5.10.0;

my @ops = qw(
    OP_NOP
    OP_ADD
    OP_SUB
    OP_DIV
    OP_MUL
    OP_PRINT
    OP_PUSH_INT
    OP_PUSH_TRUE
    OP_PUSH_FALSE
    OP_PUSH_IDENTIFIER
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

    OP_SETVARIABLE
    OP_GETVARIABLE

    OP_DEFINE_METHOD
);

my $xslate = Text::Xslate->new(
    syntax => 'TTerse',
);

print $xslate->render_string(<<'...', { ops => \@ops });
#ifndef OPS_GEN_H_
#define OPS_GEN_H_

typedef enum {
[% FOR op IN ops -%]
    [% op %],
[% END -%]
} OP_TYPE;

static const char*opcode2name[] = {
[% FOR op IN ops -%]
    "[% op %]",
[% END -%]
};

#endif // OPS_GEN_H_
...

