package Something;
use strict;
sub thing { Something->new(@_) }
our @EXPORT = qw(thing);
use Thing -base;

field color => 'blue';

sub cool {}

1;
