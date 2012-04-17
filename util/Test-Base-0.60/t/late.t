use Test::Base tests => 5;

run {};

eval {
    filters 'blah', 'blam';
};
is "$@", "";

eval {
    filters {foo => 'grate'};
};
is "$@", "";

eval {
    delimiters '***', '&&&';
};
like "$@", qr{^Too late to call delimiters\(\)};

eval {
    spec_file 'foo.txt';
};
like "$@", qr{^Too late to call spec_file\(\)};

eval {
    spec_string "my spec\n";
};
like "$@", qr{^Too late to call spec_string\(\)};

__DATA__

=== Dummy
--- foo
--- bar
