package Testorama;
use Test::Base -Base;

BEGIN {
    our @EXPORT = qw(run_orama);
}

sub run_orama {
    pass 'Testorama EXPORT ok';
}

package Test::Base::Block;

sub foofoo {
    Test::More::pass 'Test::Base::Block ok';
}

package Testorama::Filter;
use base 'Test::Base::Filter';

sub rama_rama {
    Test::More::pass 'Testorama::Filter ok';
}

package main;
# use Testorama;
BEGIN { Testorama->import }

plan tests => 3;

run_orama;

[blocks]->[0]->foofoo;

__DATA__
===
--- stuff chomp rama_rama
che!
