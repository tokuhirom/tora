package Foo;
use strict;
use Test::More tests => 20;
use lib 't';
use Something;

ok(not defined &Foo::import);
ok(defined &Foo::thing);
ok(ref(thing) eq 'Something');
ok(thing()->can('cool'));
ok(thing()->isa('Something'));
ok(thing()->isa('Thing'));
ok(thing()->isa('Spiffy'));
is(join('-', @Foo::ISA), '');
ok(not defined &Foo::field);
ok(not defined &Foo::spiffy_constructor);

ok(not defined &Something::import);
ok(defined &Something::thing);
ok(defined &Something::field);
ok(not defined &Something::spiffy_constructor);
is(join('-', @Something::ISA), 'Thing');

ok(not defined &Thing::import);
ok(defined &Thing::thing);
ok(defined &Thing::field);
ok(not defined &Thing::spiffy_constructor);
is(join('-', @Thing::ISA), 'Spiffy');
