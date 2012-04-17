use lib 't', 'lib';
package A;
use Spiffy -Base;

package B;
use Spiffy -Base;
field foo => 42;


package main;
use Test::More tests => 1;

my $a = A->new;
$a->mixin('B');
is($a->foo, 42);
