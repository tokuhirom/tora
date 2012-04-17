use Test::Base tests => 2;

delimiters qw($$$ ***);

run {
    ok(shift);
};

__END__

$$$
*** foo
this
*** bar
that

$$$

*** foo
hola
*** bar
latre
