use Test::Base tests => 7;

filters 'chomp';

is next_block->test, '1';
is next_block->test, '2';
is first_block->test, '1';
is first_block->test, '1';
is next_block->test, '2';
is next_block->test, '3';
ok not defined next_block;

__DATA__
===
--- test
1

===
--- test
2

===
--- test
3
