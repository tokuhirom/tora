use Test::Base;

__DATA__
===
--- in lines head
one
two
three
--- out
one

===
--- in lines head=2 join
one
two
three
--- out
one
two
