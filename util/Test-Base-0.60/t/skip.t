use Test::Base tests => 5;

run { pass };

is scalar(blocks), 2;

my @block = blocks;
is $block[0]->foo, "2\n";
is $block[1]->foo, "3\n";

__DATA__
=== One
--- SKIP
--- foo
1
=== Two
--- foo
2
=== Three
--- foo
3
=== Four
--- SKIP
--- foo
4
