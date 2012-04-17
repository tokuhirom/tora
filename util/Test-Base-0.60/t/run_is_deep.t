use Test::Base;

BEGIN {
    skip_all_unless_require('Test::Deep');
}

plan tests => 3;

filters 'eval';

run_is_deep qw(foo bar);

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
