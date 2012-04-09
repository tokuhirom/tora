#!/usr/bin/env perl

use strict;
use warnings;

use lib 'lib';
use App::Prove::State;
use List::Util 'sum';
use Lingua::EN::Numbers 'num2en';
use Text::Table;
use Carp;

sub minutes_and_seconds {
    my $seconds = shift;
    return ( int( $seconds / 60 ), int( $seconds % 60 ) );
}

my $state      = App::Prove::State->new( { store => '.prove' } );
my $results    = $state->results;
my $generation = $results->generation;
my @tests      = $results->tests;

my $total = sum( map { $_->elapsed } @tests );
my ( $minutes, $seconds ) = minutes_and_seconds($total);

my $num_tests = shift || 10;
my $total_tests = scalar $results->test_names;

if ( $num_tests > $total_tests ) {
    $num_tests = $total_tests;
}

my $num_word = num2en($num_tests);

my %time_for;
foreach my $test (@tests) {
    $time_for{ $test->name } = $test->elapsed;
}

my @sorted_by_time_desc
  = sort { $time_for{$b} <=> $time_for{$a} } keys %time_for;

print "Number of test programs: $total_tests\n";
print "Total runtime approximately $minutes minutes $seconds seconds\n\n";
print "\u$num_word slowest tests:\n";

my @rows;
for ( 0 .. $num_tests - 1 ) {
    my $test = $sorted_by_time_desc[$_];
    my $time = $time_for{$test};
    my ( $minutes, $seconds ) = minutes_and_seconds($time);
    push @rows => [ "${minutes}m ${seconds}s", $test, ];
}

print make_table(
    [qw/Time Test/],
    \@rows,
);

sub make_table {
    my ( $headers, $rows ) = @_;

    my @rule    = qw(- +);
    my @headers = \'| ';
    push @headers => map { $_ => \' | ' } @$headers;
    pop @headers;
    push @headers => \' |';

    unless ( 'ARRAY' eq ref $rows
        && 'ARRAY' eq ref $rows->[0]
        && @$headers == @{ $rows->[0] } )
    {
        croak(
            "make_table() rows must be an AoA with rows being same size as headers"
        );
    }
    my $table = Text::Table->new(@headers);
    $table->rule(@rule);
    $table->body_rule(@rule);
    $table->load(@$rows);
    return $table->rule(@rule),
      $table->title,
      $table->rule(@rule),
      map( { $table->body($_) } 0 .. @$rows ),
      $table->rule(@rule);
}
