use Test::Base tests => 10;

for (1..2) {
    is next_block->foo, 'This is foo';
    is next_block->bar, 'This is bar';

    while (my $block = next_block) {
        pass;
    }
}

__DATA__
=== One
--- foo chomp
This is foo
=== Two
--- bar chomp
This is bar
=== Three
=== Four
=== Five
