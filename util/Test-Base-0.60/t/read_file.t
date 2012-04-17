use Test::Base;

__END__
=== Filename is chomped automatically
--- file read_file
t/sample-file.txt
--- content
A sample of some text
in a sample file!

=== Filename is inline
--- file read_file: t/sample-file.txt
--- content
A sample of some text
in a sample file!
