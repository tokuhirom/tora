use Test::Base;

__DATA__
===
--- in lines tail
one
two
three
--- out
three

===
--- in lines tail=2 join
one
two
three
--- out
two
three
