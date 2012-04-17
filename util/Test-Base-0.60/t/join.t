use Test::Base tests => 3;

is next_block->input, 'onetwothree';
is next_block->input, 'one=two=three';
is next_block->input, "one\n\ntwo\n\nthree";

__DATA__
===
--- input lines chomp join
one
two
three

===
--- input lines chomp join==
one
two
three

===
--- input lines chomp join=\n\n
one
two
three
