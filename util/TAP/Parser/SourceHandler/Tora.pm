package TAP::Parser::SourceHandler::Tora;
 
use strict;
use vars qw($VERSION @ISA);
 
use TAP::Parser::SourceHandler     ();
use TAP::Parser::IteratorFactory   ();
use TAP::Parser::Iterator::Process ();
 
@ISA = qw(TAP::Parser::SourceHandler);
 
TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);
 
 
$VERSION = '3.23';
 
 
sub can_handle {
    my ( $class, $src ) = @_;
    my $meta = $src->meta;
 
     if ( $meta->{is_file} ) {
        my $file = $meta->{file};
        return 1 if $file->{lc_ext} eq '.tra';
    }
    return 0;
}
 
 
sub make_iterator {
    my ( $class, $source ) = @_;
    my $meta = $source->meta;
 
    my @command;
    if ( $meta->{is_hash} ) {
        @command = @{ $source->raw->{exec} || [] };
    }
    elsif ( $meta->{is_scalar} ) {
        @command = ${ $source->raw };
    }
    elsif ( $meta->{is_array} ) {
        @command = @{ $source->raw };
    }
 
    $class->_croak('No command found in $source->raw!') unless @command;
 
    $class->_autoflush( \*STDOUT );
    $class->_autoflush( \*STDERR );
 
    push @command, @{ $source->test_args || [] };
 
    return $class->iterator_class->new(
        {   command => ['./bin/tora', @command],
            merge   => $source->merge
        }
    );
}
 
 
use constant iterator_class => 'TAP::Parser::Iterator::Process';
 
# Turns on autoflush for the handle passed
sub _autoflush {
    my ( $class, $flushed ) = @_;
    my $old_fh = select $flushed;
    $| = 1;
    select $old_fh;
}
 
1;
 
