use Test::Base;

plan tests => 1 * blocks;

# A silly test instead of pod2html
for my $block (blocks) {
    is(
        uc($block->pod),
        $block->upper,
        $block->name, 
    );
}

__END__
=== Header 1 Test
--- pod
=head1 The Main Event
--- upper
=HEAD1 THE MAIN EVENT
=== List Test
--- pod
=over
=item * one
=item * two
=back
--- upper
=OVER
=ITEM * ONE
=ITEM * TWO
=BACK
