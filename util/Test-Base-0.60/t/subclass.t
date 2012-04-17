use lib 't';
use TestBass tests => 7;

eval "use Test::Base";
is "$@", '', 'ok to import parent class *after* subclass';

my @blocks = blocks;

is ref(default_object), 'TestBass';

is $blocks[0]->el_nombre, 'Test One';

ok $blocks[0]->can('feedle'), 'Does feedle method exist?';

run_is xxx => 'yyy';

run_like_hell 'thunk', qr(thunk,.*ile.*unk);

__DATA__
=== Test One
--- xxx lines foo_it join
a lion
a tiger
a liger
--- yyy
foo - a lion
foo - a tiger
foo - a liger

===
--- thunk
A thunk, a pile of junk
===
--- thunk
A thunk, a jile of punk
