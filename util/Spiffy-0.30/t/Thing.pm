package Thing;
use strict;
use Spiffy -base;
use base 'Spiffy';
our @EXPORT = qw(thing);

field volume => 11;

sub thing { Thing->new(@_) }

1;
