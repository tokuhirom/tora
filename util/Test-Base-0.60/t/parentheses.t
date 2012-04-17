use Test::Base tests => 17;

sub some_text { 'This is some text' };

my $b = first_block;
is $b->foo, $b->bar, $b->name;
is $b->foo, some_text();

run {
    my $b = shift;
    ok defined $b->foo;
    is @{[$b->foo]}, 1;
    ok length $b->foo;
};

__DATA__

=== Parens clarify section
--- (foo) some_text
--- (bar) some_text

===
--- (foo: some text

===
--- foo)
some text

=== 
--- (foo): some text

=== 
--- (foo) split join: some text

