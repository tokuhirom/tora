#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;

open my $fh, '<', 'src/parser.h';
open my $ofh, '>', 'src/token.gen.h';
print $ofh qq!static const char *token_id2name[] = {"EOF",\n!;
while (<$fh>) {
    chomp;
    my @a = split /[ \t]+/, $_;
    my $name = $a[1];
    my $no   = $a[2];
    print $ofh qq{"$name",\n};
}
print $ofh qq!};\n!;

