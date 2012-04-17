use Test::Base;

plan tests => 1 * blocks;

my @blocks = blocks;

is $blocks[0]->name, 'One Time';
is $blocks[1]->name, 'Two Toes';
is $blocks[2]->name, '';
is $blocks[3]->name, 'Three Tips';

__END__
=== One Time
=== Two Toes
--- foo
===



=== Three Tips
