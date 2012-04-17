use Test::Base;

__DATA__
===
--- words lines chomp hash
foo
42
bar
44
baz
because
--- hash eval
+{
    foo => 42,
    bar => 44,
    baz => 'because',
}
