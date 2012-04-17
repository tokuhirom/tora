use lib 'lib';
use strict;
use warnings;
use Test::More tests => 1;
use Spiffy;

my $args = Spiffy->parse_arguments();

ok(ref $args && ref($args) eq 'HASH');
