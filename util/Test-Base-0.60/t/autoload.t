use Test::Base tests => 4;

my $block = first_block;
ok((not defined &Test::Base::Block::bogus_method),
   "Method doesn't exist");
ok((not exists $block->{bogus_method}),
   "Slot really doesn't exist");
ok((not defined $block->bogus_method),
   "Method is callable");
my @list_context = $block->bogus_method;
ok @list_context == 0,
   "Returns nothing in list context";

__DATA__
=== One
--- xyz
Flavor
