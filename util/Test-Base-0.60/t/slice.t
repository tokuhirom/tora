use Test::Base;

__DATA__
===
--- in lines slice=0,2 join
one
two
three
four
five
--- out
one
two
three

===
--- in lines slice=2,3 join
one
two
three
four
five
--- out
three
four

===
--- in lines slice=1 join
one
two
three
--- out
two
