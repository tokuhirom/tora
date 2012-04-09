#!/usr/bin/perl

use warnings;
use strict;

use Test::More;

# TODO we need to have some way to find one or more alternate versions
# of perl on the smoke machine so that we can verify that the installed
# perl can be used to test against the alternate perls without
# installing the harness in the alternate perls.  Does that make sense?
#
# Example:
#  harness process (i.e. bin/prove) is perl 5.8.8.
#  subprocesses    (i.e. t/test.t) are perl 5.6.2.

my @perls;

BEGIN {
    my $perls_live_at = '/usr/local/stow/';
    @perls = grep( { -e $_ }
        map( {"$perls_live_at/perl-$_/bin/perl"} qw(5.5.4 5.6.2) ) );
    if (@perls) {
        plan( tests => scalar(@perls) * 4 );
    }
    else {
        plan( skip_all => "no perls found in '$perls_live_at'" );
    }
}

use File::Temp ();
use File::Path ();
use IPC::Run   ();

mkdir('twib') or die "cannot create 'twib' $!";

{    # create a lib
    open( my $fh, '>', 'twib/foo.pm' );
    print $fh "package twib;\nsub foo {'bar';}\n1;\n";
}

END {
    File::Path::rmtree('twib');
}

my @tests = qw(
  xt/perls/sample-tests/perl_version
);

# TODO and something with taint

# make the tests check that the perl is indeed the $perl (thus they are
# just printed tests.)
for my $perl (@perls) {

    # TODO make the API be *not* an environment variable!
    local $ENV{HARNESS_PERL} = $perl;

    my ( $in, $out, $err ) = ( undef, '', '' );
    my $ret = IPC::Run::run(
        [
            $^X, '-Ilib',
            'bin/prove', '-It/lib', '-Itwib', @tests
        ],
        \$in, \$out, \$err
    );
    ok( $ret, 'no death' );
    like( $out, qr/All tests successful/, 'success' );
    like( $out, qr/Result: PASS/,         'passed' );
    is($err, '', 'no error');
}

# vim:ts=4:sw=4:et:sta
