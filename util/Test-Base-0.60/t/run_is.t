use Test::Base;

plan tests => 7 * blocks;

run_is 'foo', 'bar';
run_is 'bar', 'baz';
run_is 'baz', 'foo';

for my $block (blocks) {
    is $block->foo, $block->bar, $block->name;
    is $block->bar, $block->baz, $block->name;
    is $block->baz, $block->foo, $block->name;
}

my @blocks = blocks;

is $blocks[0]->foo, "Hey Now\n";
is $blocks[1]->foo, "Holy Cow\n";

__END__


=== One
--- foo
Hey Now
--- bar
Hey Now
--- baz
Hey Now


=== Two
--- baz
Holy Cow
--- bar
Holy Cow
--- foo
Holy Cow
