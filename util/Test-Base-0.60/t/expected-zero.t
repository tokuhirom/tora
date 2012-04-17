use Test::Base;
plan tests => 1*blocks;

run {
    my $block = shift;
    is 0, $block->expected;
}

__END__

=== ok
--- expected chomp
0

=== oops
--- expected: 0
