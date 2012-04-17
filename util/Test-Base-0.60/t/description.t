use Test::Base tests => 6;

my @blocks = blocks;

is $blocks[0]->description, 'One Time';
is $blocks[1]->description, "This is the real description\nof the test.";
is $blocks[2]->description, '';
is $blocks[3]->description, '';
is $blocks[4]->description, 'Three Tips';
is $blocks[5]->description, 'Description goes here.';

__END__
=== One Time
=== Two Toes
This is the real description
of the test.
--- foo
bar
===

===
=== Three Tips

--- beezle
blob

===
Description goes here.
--- data
Some data
