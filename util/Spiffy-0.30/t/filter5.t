use lib 't', 'lib';
use strict;

use Test::More tests => 1;
use Spiffy -filter_save;
use Filter5;

my $result = $Spiffy::filter_result;
my $expected = do { local $/; <DATA> };
$result =~ s/\r//g;
$expected =~ s/\r//g;
is($result, $expected);

__DATA__
use strict;use warnings;my($xxx,$yyy);$xxx = sub {my $self = shift;
    $self->$xxx;
    $self->$yyy;
};
$yyy = sub {my $self = shift;
    $self->$xxx;
    $self->$yyy
};
;1;
