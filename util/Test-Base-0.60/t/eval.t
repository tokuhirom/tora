use Test::Base tests => 4;

filters 'eval';

my $block = first_block;

is ref($block->hash), 'HASH';
is ref($block->array), 'ARRAY';
is scalar(@{$block->array}), 11;
is $block->factorial, '362880';

__END__

=== Test
--- hash
{
    foo => 'bar',
    bar => 'hihi',
}
--- array
[ 10 .. 20 ]
--- factorial
my $x = 1;
$x *= $_ for (1 .. 9);
$x;
