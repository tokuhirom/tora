use Test::Base tests => 1;

is next_block->perl, <<'...';
You are a foo!
You are 1 2.
...

__DATA__
===
--- perl eval_stderr
warn "You are a foo!\n";
my $foo = 2;
print STDERR "You are 1 $foo.\n";
return 42;
