#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use autodie;

my $dat = parse();
write_file('src/vm.gen.cc', vm_gen_cc($dat));
write_file('src/ops.gen.h', ops_gen_h($dat));
write_file('src/ops.gen.cc', ops_gen_cc($dat));
write_file('src/vm.ops.inc.h', vm_ops_inc_h($dat));

sub write_file {
    my ($fname, $body) = @_;
    open my $fh, '>', $fname;
    print $fh $body;
}

sub parse {
    open my $fh, '<', 'vm.inc';
    my @ret;
    my $current_op;
    my $code;
    my $lineno = 1;
    my $start;
    while (<$fh>) {
        if (/^(OP_[A-Z_]+)\s+\{/) {
            $code = '';
            $current_op = $1;
            $start = $lineno;
        } elsif (/^\}/) {
            push @ret, [$current_op, $code, $start];
        } else {
            $code .= $_;
        }
        $lineno++;
    }
    return \@ret;
}

sub vm_gen_cc {
    my $dat = shift;

    my $ret .= <<'...';
#include "vm.h"
#include "vm.cc"
#include "value.h"
#include "value/hash.h"
#include "value/code.h"
#include "value/tuple.h"
#include "value/file.h"
#include "value/range.h"
#include "regexp.h"
#include <unistd.h>
#include <algorithm>
#include <functional>

using namespace tora;

...

    {
        for my $k (@$dat) {
            $ret .= "void VM::PP_$k->[0]() {\n";
            # $ret .= "inline void VM::PP_$k->[0]() {\n";
            $ret .= "SharedPtr<OP> op = ops->at(pc);\n";
            $ret .= "$k->[1]\n";
            $ret .= "}\n";
        }
    }

    $ret .= <<'...';

// run program
void VM::execute() {
    DBG2("************** VM::execute\n");

#ifdef __GNUC__
    static const void *JUMPTABLE [] = {
...

    for my $k (@$dat) {
        $ret .= sprintf("&&CODE_%s,", $k->[0]);
    }

    $ret .= <<'...';
    &&CODE_OP_END};

    {
        goto *JUMPTABLE[ops->at(pc)->op_type];
...

    for my $k (@$dat) {
        $ret .= "CODE_$k->[0]: {\n";
        if (0) {
            $ret .= "    printf(\"calling PP_$k->[0](%d).\\n\", ops->at(pc)->op_type);\n";
        }
        $ret .= "PP_$k->[0]();\n pc++; goto *JUMPTABLE[ops->at(pc)->op_type]; }\n";
    }

    $ret .= <<'...';
CODE_OP_END:
        goto END;
    }

#else

    for (;;) {
#ifdef DEBUG
        DBG2("[DEBUG] ");
        disasm_one(ops->at(pc));
#endif

        OP* op = ops->at(pc);
        switch (op->op_type) {
...

    for my $k (@$dat) {
        $ret .= "    case $k->[0]: { PP_$k->[0](); break; }\n";
    }

    $ret .= <<'...';
        case OP_END: { goto END; }
        }
        // dump_stack();
        pc++;
    }
#endif // __GNUC__

END:
    return;
}
...
}

sub ops_gen_h {
    my $dat = shift;

    sprintf <<'...', join(", ", map {$_->[0]} @$dat);
#ifndef OPS_GEN_H_
#define OPS_GEN_H_

namespace tora {

typedef enum {%s,OP_END} op_type_t;

extern const char*opcode2name[];

};

#endif // OPS_GEN_H_
...
}

sub ops_gen_cc {
    my $dat = shift;

    sprintf (<<'...', join(", ", map{ qq{"$_->[0]"} } @$dat));
#include "ops.gen.h"

const char*tora::opcode2name[] = {%s,"OP_END"};
...
}

sub vm_ops_inc_h {
    my $dat = shift;

    join('', map { qq{void PP_$_->[0]();\n} } @$dat);
}

