use lib 't', 'lib';
use Spiffy ();
package A;
sub _role_a { qw(a1 a2 a3) }
sub a1 {'a1' }
sub a2 {'a2' }
sub a3 {'a3' }
sub _role_A { qw(A1 A2 A3) }
sub A1 {'A1' }
sub A2 {'A2' }
sub A3 {'A3' }
sub _role_aA { qw(:a :A foo) }
sub foo {'foo'}

package BB;
use base 'A';

package X;
use mixin A => qw(:a !a2);

package X2;
use mixin BB => qw(:a !a2);

package X3;
use mixin A => qw(!:A A2);

package X4;
use mixin A => qw(:aA !a1 !a1 !A1);

package X5;
use mixin A => qw(!:a !:A);

package main;
use Test::More tests => 32;

ok(X->can('a1'));
ok(not X->can('a2'));
ok(X->can('a3'));
ok(not X->can('A1'));
is(X->a1, 'a1');
is(X->a3, 'a3');

ok(X2->can('a1'));
ok(not X2->can('a2'));
ok(X2->can('a3'));
ok(not X2->can('A1'));
is(X2->a1, 'a1');
is(X2->a3, 'a3');

ok(X3->can('a1'));
ok(X3->can('a2'));
ok(X3->can('a3'));
ok(not X3->can('A1'));
ok(X3->can('A2'));
ok(not X3->can('A3'));

ok(not X4->can('a1'));
ok(X4->can('a2'));
ok(X4->can('a3'));
ok(not X4->can('A1'));
ok(X4->can('A2'));
ok(X4->can('A3'));
ok(X4->can('foo'));

ok(not X5->can('a1'));
ok(not X5->can('a2'));
ok(not X5->can('a3'));
ok(not X5->can('A1'));
ok(not X5->can('A2'));
ok(not X5->can('A3'));
ok(X5->can('foo'));
