use strict;
use 5.10.0;

sub fib {
    if ($_[0]<2) {
        return $_[0];
    } else {
        return fib($_[0]-2) + fib($_[0]-1);
    }
}

say(fib(39));

