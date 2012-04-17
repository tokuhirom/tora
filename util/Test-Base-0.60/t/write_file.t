use t::BaseTest tests => 4;

my $file = 't/output/foo.txt';

ok not(-e $file), "$file doesn't already exist";

first_block;

ok -e $file, "$file exists";

open my $fh, $file
  or die "Can't open '$file' for input:\n$!";
is join('', <$fh>),
   "One two\nBuckle my shoe\n",
   '$file content is right';

is first_block->poem, $file, 'Returns file name';

__END__

===
--- poem write_file=t/output/foo.txt
One two
Buckle my shoe
