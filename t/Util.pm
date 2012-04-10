package t::Util;
use strict;
use warnings;
use utf8;
use Test::More;
use File::Temp qw/ tmpnam/;
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
        if ($^O eq 'MSWin32') {
            my $in = tmpnam();
            open my $f, '>:raw', $in;
            print $f $code;
            close $f;
            my $pid = open3(undef, $out, $err, "type $in | .\\bin\\tora");
            my $ret = join('', <$out>);
            my $err_ret = join('', <$err>);
            (my $name = $code) =~ s/\n/ /g;
            waitpid($pid, 0);
            $ret =~ s!\r!!g; # TODO(mattn): remove workaround.
            $err_ret =~ s!\r!!g; # TODO(mattn): remove workaround.
			my $status = ($? >> 8);
            ::is($status, $exit_status, "exit status is $exit_status");
            ::is($ret, $out_expected, $name);
            $err_expected = '' unless defined $err_expected;
            ::is($err_ret, $err_expected, "STDERR: $name");
            unlink $in;
        } else {
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
        }
    };
    Test::More->export_to_level(1);
}


1;

