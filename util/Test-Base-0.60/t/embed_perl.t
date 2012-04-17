# This feature allows you to put a Perl section at the top of your
# specification, between <<< and >>>. Not making this an official
# feature yet, until I decide whether I like it.

use Test::Base tests => 2;

run_is;

sub reverse { join '', reverse split '', shift }

__DATA__

<<< delimiters '+++', '***'; 
filters 'chomp';
>>>


+++ One
*** x reverse
123*
*** y
*321

+++ Two
*** x reverse
abc
*** y
cba
