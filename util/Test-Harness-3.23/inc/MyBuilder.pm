package MyBuilder;

BEGIN {
    require Module::Build;
    @ISA = qw(Module::Build);
}

# Test with Test::Harness
sub ACTION_test_with_harness {
    my $self = shift;

    $self->SUPER::ACTION_test(@_);
}

# Test with TAP::Harness instead of Test::Harness
sub ACTION_test {
    my $self = shift;

    $self->depends_on('code');

    my $tests = $self->find_test_files;
    unless (@$tests) {
        $self->log_info("No tests defined.\n");
        return;
    }

    # TODO verbose and stuff

    require TAP::Harness;
    my $harness = TAP::Harness->new( { lib => 'blib/lib' } );
    my $aggregator = $harness->runtests(@$tests);
    die "Failed!\n" if $aggregator->has_problems;
}

sub ACTION_testprove {
    my $self = shift;
    $self->depends_on('code');
    exec( $^X, '-Iblib/lib', 'bin/prove', '-b', '-r', 't' );
}

sub ACTION_testleaks {
    my $self = shift;
    $self->depends_on('code');
    exec( $^X, '-MDevel::Leak::Object=GLOBAL_bless', '-Iblib/lib',
        'bin/prove', '-b', '-r', 't'
    );
}

sub ACTION_testreference {
    my $self = shift;
    $self->depends_on('code');
    my $ref = 'reference/Test-Harness-2.64';
    exec( $^X,
        ( -e $ref ? ( "-I$ref/lib", "$ref/bin/prove" ) : qw(-S prove) ),
        '-Iblib/lib', '-r', 't'
    );
}

sub ACTION_testauthor {
    my $self = shift;
    $self->test_files('xt/author');
    $self->ACTION_test;
}

sub ACTION_critic {
    exec(
        qw(perlcritic -1 -q -profile perlcriticrc
          bin/prove lib/), glob('t/*.t')
    );
}

sub ACTION_tags {
    exec(
        qw(ctags -f tags --recurse --totals
          --exclude=blib
          --exclude=.svn
          --exclude='*~'
          --languages=Perl
          t/ lib/ bin/prove
          )
    );
}

sub ACTION_tidy {
    my $self = shift;

    my @extra = qw(
      Build.PL
      Makefile.PL
      bin/prove
    );

    my %found_files = map {%$_} $self->find_pm_files,
      $self->_find_file_by_type( 'pm', 't' ),
      $self->_find_file_by_type( 'pm', 'inc' ),
      $self->_find_file_by_type( 't',  't' );

    my @files = (
        keys %found_files,
        map { $self->localize_file_path($_) } @extra
    );

    for my $file (@files) {
        system( 'perltidy', '-b', $file );
        unlink("$file.bak") if $? == 0;
    }
}

my @profiling_target = qw( -Mblib bin/prove --timer t/regression.t );

sub ACTION_dprof {
    system( $^X, '-d:DProf', @profiling_target );
    exec(qw( dprofpp -R ));
}

sub ACTION_smallprof {
    system( $^X, '-d:SmallProf', @profiling_target );
    open( FH, 'smallprof.out' ) or die "Can't open smallprof.out: $!";
    @rows = grep {/\d+:/} <FH>;
    close FH;

    @rows = reverse
      sort { ( split( /\s+/, $a ) )[2] <=> ( split( /\s+/, $b ) )[2] } @rows;
    @rows = @rows[ 0 .. 30 ];
    print join( '', @rows );
}

sub read_manifest {
    my ( $self, $file, $into ) = @_;
    open my $fh, '<', $file or die "Can't read $file: $!";
    while (<$fh>) {
        chomp;
        s/\s*#.*//;
        $into->{$_}++ if length $_;
    }
}

sub ACTION_manifest {
    my ( $self, @args ) = @_;
    $self->SUPER::ACTION_manifest(@args);
    my $stash = {};
    my $mc    = 'MANIFEST.CUMMULATIVE';
    $self->read_manifest( $mc,        $stash );
    $self->read_manifest( 'MANIFEST', $stash );
    open my $fh, '>', $mc or die "Can't write $mc: $!";
    print $fh "$_\n" for sort keys %$stash;
}

1;
