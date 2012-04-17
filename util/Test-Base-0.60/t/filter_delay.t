# Each filter should have access to blocks/block internals.
use Test::Base;

filters qw(chomp lower);
filters_delay;

plan tests => 8 * blocks;

for my $block (blocks) {
    ok not($block->is_filtered);
    unlike $block->section, qr/[a-z]/;
    like $block->section, qr/^I L/;
    like $block->section, qr/\n/;
    $block->run_filters;
    ok $block->is_filtered;
    like $block->section, qr/[a-z]/;
    like $block->section, qr/^i l/;
    unlike $block->section, qr/\n/;
}

sub lower { lc }

__DATA__
=== One
--- section
I LIKE IKE

=== One
--- section
I LOVE LUCY
