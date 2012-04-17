use Test::Base tests => 4;

my ($block1, $block2) = blocks;

is $block1->foo, "line 1\nline 2\n";
is $block1->bar, "line1\nline2\n";
is $block2->foo, "aaa\n\nbbb\n";
is $block2->bar, "\nxxxx\n\nyyyy\n\n";


__END__

=== One

--- foo
line 1
line 2

--- bar

line1
line2

=== Two

--- bar -trim

xxxx

yyyy

--- foo

aaa

bbb


