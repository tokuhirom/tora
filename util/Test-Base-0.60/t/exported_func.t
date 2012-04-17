package Testfunc;
use Test::Base -Base;

BEGIN {
    our @EXPORT = qw(func_with_args);
}

sub func_with_args() {
    (my ($self), @_) = find_my_self(@_);
    return @_;
}

package main;
BEGIN { Testfunc->import }

plan tests => 1;

my @ret = func_with_args(1, 2, 3);

is_deeply \@ret, [ 1, 2, 3 ];

