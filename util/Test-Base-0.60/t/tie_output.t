use Test::Base tests => 3;

my $out = "Stuff\n";
my $err = '';

tie_output(*STDOUT, $out);
tie_output(*STDERR, $err);

warn "Keep out!\n";

print "The eagle has landed\n";

is $out, "Stuff\nThe eagle has landed\n";

print "This bird had flown\n";

is $out, "Stuff\nThe eagle has landed\nThis bird had flown\n";

print STDERR "You 'lil rascal...\n";

is $err, "Keep out!\nYou 'lil rascal...\n";
