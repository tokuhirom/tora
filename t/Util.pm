package t::Util;
use strict;
use warnings;
use utf8;
use Test::More;
use File::Temp;
use IPC::Open3;
use Symbol 'gensym';
use POSIX;

sub import {
    strict->import;
    warnings->import;
    no strict 'refs';
    *{caller(0) . "::run_is"} = sub {
        note("L: " . (caller(0))[2]);
        my ($code, $out_expected, $err_expected, $exit_status) = @_;
        $exit_status ||= 0;

        my ($in, $out, $err);
        $err = gensym;
        my $pid = open3($in, $out, $err, './bin/tora');
        print $in $code;
        close $in;
        my $ret = join('', <$out>);
        my $err_ret = join('', <$err>);
        (my $name = $code) =~ s/\n/ /g;
        waitpid($pid, 0);
        ::ok(POSIX::WIFEXITED($?), 'EXITED');
        ::is(POSIX::WEXITSTATUS($?), $exit_status, "exit status is $exit_status");
        ::is($ret, $out_expected, $name);
        $err_expected = '' unless defined $err_expected;
        ::is($err_ret, $err_expected, "STDERR: $name");
    };
    Test::More->export_to_level(1);
}


1;

