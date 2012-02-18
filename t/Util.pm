package t::Util;
use strict;
use warnings;
use utf8;
use Test::More;
use File::Temp;
use IPC::Open3;
use Symbol 'gensym';

sub import {
    strict->import;
    warnings->import;
    no strict 'refs';
    *{caller(0) . "::run_is"} = sub {
        note("L: " . (caller(0))[2]);
        my ($code, $out_expected, $err_expected) = @_;
        my ($in, $out, $err);
        $err = gensym;
        open3($in, $out, $err, './tora');
        print $in $code;
        close $in;
        my $ret = join('', <$out>);
        my $err_ret = join('', <$err>);
        (my $name = $code) =~ s/\n/ /g;
        ::is($ret, $out_expected, $name);
        if (defined $err_expected) {
            ::is($err_ret, $err_expected, "STDERR: $name");
        }
    };
    Test::More->export_to_level(1);
}


1;

