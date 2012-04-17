use lib 'lib';
use strict;
use warnings;

package Alpha;
use Spiffy -Base;

sub three {
    print "ok 6\n";
}

package Foo;
use base 'Alpha';

sub one {
    super;
    print "ok 2\n";
}

sub two {
    print "ok 4\n";
}

package Bar;
use base 'Foo';

sub one {
    super;
    print "ok 3\n";
}

sub two {
    super;
    print "ok 5\n";
}

package Baz;
use base 'Bar';

sub one {
    print "ok 1\n";
    super;
}

sub two {
    super;
    print "not ok 6\n";
}

sub three {
    super;
    print "ok 7\n";
}

package main;
use strict;
print "1..7\n";
Baz->new->one;
Bar->new->two;
Baz->new->three;
