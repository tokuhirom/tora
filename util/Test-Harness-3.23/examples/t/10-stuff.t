#!/usr/bin/perl -wT
use strict;

use Test::More qw/no_plan/;

ok 1, 'this test passes';
is_deeply [2], [3], 'this is_deeply test fails';
SKIP: {
    skip 'testing skip', 2 if 1;
    ok 1;
    ok 1;
}
TODO: {
    local $TODO = 'this is a TODO test';
    ok 0, 'This should succeed';
    ok 1, 'This should fail';
}
