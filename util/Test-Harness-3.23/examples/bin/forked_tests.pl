#!/usr/bin/perl

# Run tests in parallel.  This just allows you to check that your tests
# are roughly capable of running in parallel.  It writes output to a
# tree in /tmp.
# From: Eric Wilhelm @ ewilhelm at cpan.org

use warnings;
use strict;

use File::Basename ();
use File::Path     ();
use List::Util     ();

my @tests = @ARGV;

#@tests = List::Util::shuffle(@tests);

use POSIX ();

my %map;
my $i = 0;

my $jobs = 9;    # scalar(@tests); # if you like forkbombs
my @running;

while (@tests) {
    if ( $jobs == @running ) {
        my @list;
        while ( my $pid = shift(@running) ) {
            if ( waitpid( $pid, POSIX::WNOHANG() ) > 0 ) {
                warn ' ' x 25 . "done $map{$pid}\n";
                next;
            }
            push( @list, $pid );
        }

        #warn "running ", scalar(@list);
        @running = @list;
        next;
    }
    my $test = shift(@tests);
    defined( my $pid = fork ) or die;
    $i++;
    if ($pid) {
        push( @running, $pid );
        $map{$pid} = $test;
        print "$test\n";
    }
    else {
        my $dest_base = '/tmp';
        my $dest_dir  = File::Basename::dirname("$dest_base/$test");
        unless ( -d $dest_dir ) {
            File::Path::mkpath($dest_dir) or die;
        }

        $| = 1;
        open( STDOUT, '>', "$dest_base/$test.out" ) or die;
        open( STDERR, '>', "$dest_base/$test.err" ) or die;
        exec( $^X, '-Ilib', $test );
    }
}

my $v = 0;
until ( $v == -1 ) {
    $v = wait;
    ( $v == -1 ) and last;
    $?           and warn "$map{$v} ($v) no happy $?";
}
print "bye\n";

# vim:ts=2:sw=2:et:sta
