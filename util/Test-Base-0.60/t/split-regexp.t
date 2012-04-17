use Test::Base;

__DATA__
===
--- (xxx) chomp split=// reverse join
one
two
--- (yyy) chomp
owt
eno

===
--- (xxx) split=/[XY]/ join=-: oneXtwoYthree
--- (yyy): one-two-three

===
--- (xxx) split join=-: one two three
--- (yyy): one-two-three
