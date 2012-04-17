BEGIN { eval("use Test::Tester") }

use Test::Base;

BEGIN {
    skip_all_unless_require('Test::Tester');
    skip_all_unless_require('Test::Deep');
}

plan tests => 2;

my $a = {};
my $b = bless {}, 'Foo';
my $name = "is_deep works on non vs blessed hashes";
my ($dummy, @results) = Test::Tester::run_tests(
    sub {
        is_deep($a, $b, $name);
    },
    {
        ok => 0,
        name => $name,
    },
);

is($results[0]->{ok}, 0, "Test did not match");
is($results[0]->{name}, $name, "Test name is correct");

