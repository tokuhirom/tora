#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use 5.010000;
use autodie;
use Test::More;
use File::Temp;
use IPC::Open3;
use Symbol 'gensym';
use POSIX;

for my $src (glob('./t/tra/*.tra')) {
    subtest $src => sub {
        my ($in, $out, $err);
        $err = gensym;
        my $pid = open3($in, $out, $err, "valgrind ./tora $src");
        close $in;
        my $ret = join('', <$out>);
        my $err_ret = join('', <$err>);
        waitpid($pid, 0);

        ::ok(POSIX::WIFEXITED($?), 'EXITED');
        ::is(POSIX::WEXITSTATUS($?), 0, "exit status is 0");
        ::like($err_ret, qr/no leaks are possible/);
        ::unlike($err_ret, qr/Conditional jump/);
    };
}

done_testing;

