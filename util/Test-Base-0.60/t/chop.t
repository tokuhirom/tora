use Test::Base;

filters qw(norm trim chomp);

plan tests => 1 * blocks;

my $c = next_block;
is_deeply $c->input, $c->output;

$c = next_block;
is $c->input, $c->output;

__END__
===
--- input lines chomp chop array
one
two
three
--- output eval
[qw(on tw thre)]


===
--- input chomp chop
one
two
three
--- output eval
"one\ntwo\nthre"

