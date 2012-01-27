package t::Util;
use strict;
use warnings;
use utf8;
use Test::More;
use File::Temp;

sub import {
    strict->import;
    warnings->import;
    no strict 'refs';
    *{caller(0) . "::run_is"} = sub {
        my ($code, $expected) = @_;
        my $tmp = File::Temp->new(UNLINK => 1);
        print $tmp $code;
        my $ret = `./tora < $tmp`;
        (my $name = $code) =~ s/\n/ /g;
        ::is($ret, $expected, $name);
    };
    Test::More->export_to_level(1);
}


1;

