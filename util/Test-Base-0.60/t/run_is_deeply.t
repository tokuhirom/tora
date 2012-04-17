use Test::Base tests => 3;

filters 'eval';

run_is_deeply qw(foo bar);

run {
    my $block = shift;
    ok ref $block->foo;
    ok ref $block->bar;
};

__DATA__
=== Test is_deeply
--- foo
{ foo => 22, bar => 33 }
--- bar
{ bar => 33, foo => 22 }
