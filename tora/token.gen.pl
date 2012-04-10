#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;

open my $fh, '<', 'tora/parser.h';

open my $hhh, '>:raw', 'tora/token.gen.h';
print $hhh <<'...';
#ifndef TORA_TOKENGEN_H_
#define TORA_TOKENGEN_H_

namespace tora {

extern const char *token_id2name[];

};

#endif
...

open my $ofh, '>:raw', 'tora/token.gen.cc';
binmode $ofh;
print $ofh qq!#include "token.gen.h"\n!;
print $ofh qq!const char *tora::token_id2name[] = {"EOF",\n!;
while (<$fh>) {
    chomp;
    my @a = split /[ \t]+/, $_;
    my $name = $a[1];
    my $no   = $a[2];
    print $ofh qq{"$name",\n};
}
print $ofh qq!};\n!;
close $ofh;
