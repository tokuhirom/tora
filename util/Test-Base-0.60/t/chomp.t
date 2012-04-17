use Test::Base;

filters qw(norm trim chomp);

plan tests => 1 * blocks;

my @blocks = blocks;

is $blocks[0]->input, "I am the foo";
is $blocks[1]->input, "One\n\nTwo\n\nThree";
is $blocks[2]->input, "Che!\n";

__END__
===
--- input
I am the foo
===
--- input

One

Two

Three

===
--- input chomp -chomp
Che!

