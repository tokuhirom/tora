Control Flow
============

if
--

 * Parens for 'if' is not needed.

Simple if stmt::

    if 3 > 0 {
        ...
    }

With elsif, else::

    if 3 > 0 {
        ...
    } elsif 400 > 20 {
        ...
    } else {
        ...
    }

for
---

C style for statement::

    for (my $i=0; $i<100; $i++) {
        ...
    }

for-each style for statement::

    for (my $v in [1,2,3]) {
        ...
    }

while
-----

while statement::

    while (true) {
        ...
    }

