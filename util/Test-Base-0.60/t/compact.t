use Test::Base;

plan tests => 1 + 1 * blocks;

filters { that => 'chomp' };

run_is this => 'that';

run sub {
    my $block = shift;
    my $value = $block->value or return;
    is $value, 'this', $block->name;
};

my $bad_spec = <<'...';
===
--- bad: real content
bogus
stuff
--- xxx
yyy
...
my $tb = Test::Base->new->spec_string($bad_spec);
eval { $tb->blocks };
like "$@",
     qr"Extra lines not allowed in 'bad' section",
     'Bad spec fails';


sub upper { uc($_) }

__DATA__

=== Basic compact form
--- (this): there is foo
--- (that)
there is foo

=== Filters work
--- (this) upper: too high to die
--- (that)
TOO HIGH TO DIE

=== Can have no value
--- (this):   
--- (that)

=== Can have ': ' in value
--- (this) : foo: bar
--- (that) chop
foo: bart

=== Test trailing blank lines are ok
--- (value): this


