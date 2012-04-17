use Test::More tests => 1;

use lib 't';

eval <<'...';
package Foo;
use base 'NonSpiffy';
...

is $@, '';
