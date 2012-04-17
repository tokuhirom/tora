use lib 't', 'lib';
use strict;

use Test::More tests => 1;
use Spiffy '-filter_save';
use Filter4;

my $result = $Spiffy::filter_result;
my $expected = do { local $/; <DATA> };
$result =~ s/\r//g;
$expected =~ s/\r//g;
is($result, $expected);

__DATA__
use strict;use warnings;my($bar,$bam);# comment

sub foo {my $self = shift;
    my $x = $self->$bar;
}

sub one {my $self = shift; }
sub uno {my $self = shift;}
$bar = sub {my $self = shift;
    if (1) {
        my $y = 1;
    }
};

sub baz {
    my $z = 1;
}

$bam = sub {my $self = shift;
    $self->$bar(42);
};
;1;
