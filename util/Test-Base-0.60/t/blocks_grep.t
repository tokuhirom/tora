use Test::Base;

my $plan = 1 * blocks('foo') + 3;

plan tests => $plan;

is $plan, 5, 'Make sure plan adds up';

for my $block (blocks('foo')) {
    is $block->foo,
       exists($block->{bar}) ? $block->bar : 'no bar';
}

eval { blocks(foo => 'bar') };
like "$@",
     qr{^Invalid arguments passed to 'blocks'};

run_is foo => 'bar';

__DATA__

===
--- bar
excluded

===
--- foo
included
--- bar
included

===
--- foo chomp
no bar
