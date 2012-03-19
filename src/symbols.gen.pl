#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;
use Text::MicroTemplate qw(render_mt);

my $dat = [
    ['SYMBOL_DESTROY'      => 'DESTROY'],

    ['SYMBOL_OBJECT_CLASS'  => 'Object'],
    ['SYMBOL_ARRAY_CLASS'  => 'Array'],
    ['SYMBOL_INT_CLASS'    => 'Int'],
    ['SYMBOL_DOUBLE_CLASS'    => 'Double'],
    ['SYMBOL_HASH_CLASS'    => 'Hash'],
    ['SYMBOL_CALLER_CLASS' => 'Caller'],
    ['SYMBOL_CODE_CLASS' => 'Code'],
    ['SYMBOL_FILE_CLASS' => 'File'],
    ['SYMBOL_STRING_CLASS' => 'String'],
    ['SYMBOL_SYMBOL_CLASS' => 'Symbol'],
    ['SYMBOL_METACLASS_CLASS' => 'MetaClass'],
    ['SYMBOL_BYTES_CLASS' => 'Bytes'],
];

open my $hfh, '>', 'src/symbols.gen.h';
print $hfh render_mt(<<'...', $dat);
#ifndef TORA_SYMBOLS_GEN_H_
#define TORA_SYMBOLS_GEN_H_

namespace tora {

? my $id = 0;
? for (@{$_[0]}) {
const int <?= $_->[0] ?> = <?= $id++ ?>;
? }

};

#endif // TORA_SYMBOLS_GEN_H_
...
close $hfh;


open my $cfh, '>', 'src/symbols.gen.cc';
print $cfh render_mt(<<'...', $dat);
#include "symbol_table.h"

using namespace tora;

SymbolTable::SymbolTable() : refcnt(0), counter(0) {
? for (@{$_[0]}) {
    this->get_id("<?= $_->[1] ?>"); // <?= $_->[0] ?>
? }
}
...
close $cfh;

