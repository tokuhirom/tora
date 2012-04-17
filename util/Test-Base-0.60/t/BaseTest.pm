package t::BaseTest;
use Test::Base -Base;

use File::Path qw(rmtree);
rmtree('t/output');

