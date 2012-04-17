use Test::Base tests => 1;

is_deeply first_block->foo, [qw(one two three)];

__DATA__


=== Create an array reference
--- foo lines chomp array
one
two
three
