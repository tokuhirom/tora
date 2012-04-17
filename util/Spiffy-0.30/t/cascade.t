use lib 'lib';

package Foo;
use strict;
use Spiffy -base;
use Cwd;
our @EXPORT = qw(cwd);

package Bar;
use strict;
Foo->base;
our @EXPORT = qw(doodle);
sub doodle {}
sub poodle {}

package Baz;
use strict;
Bar->base;

package main;
use strict;
use Test::More tests => 12;

ok(not defined &Foo::import);
ok(defined &Foo::cwd);
ok(not defined &Foo::doodle);
ok(not defined &Foo::poodle);

ok(not defined &Bar::import);
ok(defined &Bar::cwd);
ok(defined &Bar::doodle);
ok(defined &Bar::poodle);

ok(not defined &Baz::import);
ok(defined &Baz::cwd);
ok(defined &Baz::doodle);
ok(not defined &Baz::poodle);
