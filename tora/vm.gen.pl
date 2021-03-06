#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use autodie;
use Text::MicroTemplate qw(render_mt build_mt);

my $dat = parse();
write_file('tora/vm.gen.cc', vm_gen_cc($dat));
write_file('tora/ops.gen.h', ops_gen_h($dat));
write_file('tora/ops.gen.cc', ops_gen_cc($dat));
write_file('tora/vm.ops.inc.h', vm_ops_inc_h($dat));

sub write_file {
    my ($fname, $body) = @_;
    unlink $fname if -f $fname;
    open my $fh, '>:raw', $fname;
    print $fh $body;
    close $fh;
    chmod 0444, $fname if $^O ne 'MSWin32'; # win32 sucks.
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
        } elsif (m{^//}) {
            # nop
        } else {
            $code .= $_;
        }
        $lineno++;
    }
    close $fh;
    return \@ret;
}

sub vm_gen_cc {
    my $dat = shift;

    return build_mt(template => <<'...', escape_func => sub { $_[0] })->($dat);
#include "vm.h"
#include "frame.h"
#include "value.h"
#include "peek.h"
#include "operator.h"
#include "disasm.h"
#include "value/hash.h"
#include "value/code.h"
#include "value/tuple.h"
#include "value/file.h"
#include "value/range.h"
#include "value/symbol.h"
#include "value/regexp.h"
#include "value/object.h"
#include "value/exception.h"
#include "value/class.h"
#include "value/bool.h"
#include "value/undef.h"
#include "value/double.h"
#include "value/bool.h"
#include "symbols.gen.h"

#include <unistd.h>
#include <algorithm>
#include <functional>
#include <sys/stat.h>

using namespace tora;
? for my $k (@{$_[0]}) {
inline void VM::PP_<?= $k->[0] ?>() {
// #line <?= $k->[2]+1 ?> "vm.inc"
<?= $k->[1] ?>
}
? }

? for (['execute_normal', 0], ['execute_trace', 1]) {
?  my ($method, $with_trace) = @{$_};
// run program
void VM::<?= $method ?>() {
    DBG2("************** VM::execute\n");

#ifdef __GNUC__
    static const void *JUMPTABLE [] = {
? for my $k (@{$_[0]}) {
        <?= sprintf("&&CODE_%s,", $k->[0]); ?>
?   }
        &&CODE_OP_END
    };

    {
        goto *JUMPTABLE[ops->op_type_at(pc)];

? for my $k (@{$_[0]}) {
        CODE_<?= $k->[0] ?>: {
?       if ($with_trace) {
            assert(ops);
            printf("[%03d] calling PP_<?= $k->[0] ?>(%d).", pc, ops->at(pc)->op_type);
            switch (ops->at(pc)->op_type) {
            case OP_PUSH_INT:
                printf(" INT:%d", ops->at(pc)->operand.int_value);
                break;
            case OP_GETCLOSURE:
                printf(" LEVEL :%d", ops->at(pc)->int_operand_high());
                printf(" NO :%d", ops->at(pc)->int_operand_low());
                break;
            default:
                // I don't know how to explain this op code.
                break;
            }
            printf("\n");
?       }
            PP_<?= $k->[0] ?>();
            pc++;
            goto *JUMPTABLE[ops->op_type_at(pc)];
        }
? }

CODE_OP_END:
        goto END;
    }

#else

    for (;;) {
#ifdef DEBUG
        DBG2("[DEBUG] ");
        disasm_one(ops->at(pc));
#endif
?       if ($with_trace) {
            printf("calling %d.\n", ops->at(pc)->op_type);
?       }

        OP* op = ops->at(pc);
        switch (op->op_type) {
? for my $k (@{$_[0]}) {
        case <?= $k->[0] ?>: { PP_<?= $k->[0] ?>(); break; }
? }
        case OP_END: { goto END; }
        }
        pc++;
    }
#endif // __GNUC__

END:
    return;
}
? } # end for
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

