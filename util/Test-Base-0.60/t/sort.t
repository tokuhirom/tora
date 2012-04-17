use Test::Base;

__DATA__
=== Can sort a list
--- (in) split sort join=-: foo bar baz
--- out: bar-baz-foo

=== Can sort backwards
--- (in) split sort reverse join=-: foo bar baz
--- out: foo-baz-bar


