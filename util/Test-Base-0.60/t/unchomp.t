use Test::Base tests => 1;

filters qw(norm trim chomp);

is next_block->input, "on\ntw\nthre\n";

__END__
===
--- input lines chomp chop unchomp join
one
two
three
