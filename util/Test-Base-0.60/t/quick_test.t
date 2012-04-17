use Test::Base;

__DATA__
=== Compare strings
--- in split sort join=\s: ccc bbb aaa
--- out: aaa bbb ccc

=== Compare deeply
--- in eval: [1, 2, 3]
--- out eval Reverse: [3, 2, 1]

=== Compare like
--- in: You are here
--- out regexp: ere$

