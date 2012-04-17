use Test::Base;

__DATA__
=== Prepend lines before lines
--- (in) lines prepend=---\n join
one
two
three
--- (out)
---
one
---
two
---
three


=== Prepend chars before lines
--- (in) lines chomp prepend=--- join=\n
one
two
three
--- (out) chomp
---one
---two
---three


=== Prepend to a multline string
--- (in) prepend=---
one
two
three
--- (out)
---one
two
three

