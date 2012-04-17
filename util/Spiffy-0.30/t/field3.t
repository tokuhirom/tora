use lib 't', 'lib';
use strict;
use warnings;

package Foo;
use Spiffy -Base;
my $test1 = field test1 => [];
my $test2 = field test2 => {};
my $test3 = field test3 => [1..4];
my $test4 = field test4 => {1..4};
my $test5 = field test5 => -weaken;
my $test6 = field test6 => -init => '$self->setup(@_)';
my $test7 = field test7 => -weak => -init => '$self->setup(@_)';

package main;
use Test::More tests => 7;

my @expected = map { s/\r//g; $_ } split /\.\.\.\r?\n/, join '', <DATA>;

my $i = 1;
for my $expected (@expected) {
    is(eval '$test' . $i++, $expected);    
}

__DATA__
sub {
  $_[0]->{test1} = []
    unless exists $_[0]->{test1};
  return $_[0]->{test1} unless $#_ > 0;
  $_[0]->{test1} = $_[1];
  return $_[0]->{test1};
}
...
sub {
  $_[0]->{test2} = {}
    unless exists $_[0]->{test2};
  return $_[0]->{test2} unless $#_ > 0;
  $_[0]->{test2} = $_[1];
  return $_[0]->{test2};
}
...
sub {
  $_[0]->{test3} = [
          1,
          2,
          3,
          4
        ]

    unless exists $_[0]->{test3};
  return $_[0]->{test3} unless $#_ > 0;
  $_[0]->{test3} = $_[1];
  return $_[0]->{test3};
}
...
sub {
  $_[0]->{test4} = {
          '1' => 2,
          '3' => 4
        }

    unless exists $_[0]->{test4};
  return $_[0]->{test4} unless $#_ > 0;
  $_[0]->{test4} = $_[1];
  return $_[0]->{test4};
}
...
sub {
  $_[0]->{test5} = '-weaken'

    unless exists $_[0]->{test5};
  return $_[0]->{test5} unless $#_ > 0;
  $_[0]->{test5} = $_[1];
  return $_[0]->{test5};
}
...
sub {
  return $_[0]->{test6} = do { my $self = $_[0]; $self->setup(@_) }
    unless $#_ > 0 or defined $_[0]->{test6};
  return $_[0]->{test6} unless $#_ > 0;
  $_[0]->{test6} = $_[1];
  return $_[0]->{test6};
}
...
sub {
  return do {
    $_[0]->{test7} = do { my $self = $_[0]; $self->setup(@_) };
    Scalar::Util::weaken($_[0]->{test7}) if ref $_[0]->{test7};
    $_[0]->{test7};
  } unless $#_ > 0 or defined $_[0]->{test7};
  return $_[0]->{test7} unless $#_ > 0;
  $_[0]->{test7} = $_[1];
  Scalar::Util::weaken($_[0]->{test7}) if ref $_[0]->{test7};
  return $_[0]->{test7};
}
