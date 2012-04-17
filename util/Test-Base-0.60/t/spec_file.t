use Test::Base;

filters 'chomp';
spec_file 't/spec2';

plan tests => 3 * blocks;

run {
    my $block = shift;
    is ref($block), 'Test::Base::Block';
};

my @blocks = blocks;

is($blocks[0]->foo, 1);
is($blocks[0]->bar, 2);
is($blocks[1]->foo, 3);
is($blocks[1]->bar, 4);
