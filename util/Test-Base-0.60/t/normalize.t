use Test::Base tests => 4;

spec_file 't/dos_spec';

my @blocks = blocks;

is $blocks[0]->Foo, "Line 1\n\nLine 2\n";
is $blocks[0]->Bar, "Line 3\nLine 4";
is $blocks[1]->Foo, "Line 5\n\nLine 6\n";
is $blocks[1]->Bar, "Line 7\nLine 8";
