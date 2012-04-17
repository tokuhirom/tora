use Test::Base;

__DATA__
===
--- text
one fish
two fish
red fish
blue fish
--- re regexp=
one fish
two fish
red fish
blue fish

===
--- text
One Fish
Two Fish
Red Fish
Blue Fish
--- re regexp=im
^one fish
^two fish
^red fish
^blue fish

===
--- text
One Fish
Two Fish
Red Fish
Blue Fish
--- re regexp
\A^one\ fish\n
^two\ fish.
^red\ fish.
^blue\ fish\n\z

