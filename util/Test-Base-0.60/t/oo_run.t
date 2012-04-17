use Test::Base;

my $blocks = Test::Base->new;
$blocks->delimiters(qw(%%% ***))->filters('lower');

plan tests => 3 * $blocks->blocks;

$blocks->run(sub {
    my $block = shift;
    is $block->foo, $block->bar, $block->name;
});

$blocks->run_is('foo', 'bar');
$blocks->run_like('foo', qr{x});

sub lower { lc }

__DATA__
%%% Test
*** foo
xyz
*** bar
XYZ
