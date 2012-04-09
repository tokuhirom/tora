class Dir
==========

Directory class.
 

Dir.new(Str $directory) : Dir
Open directory named $directory and returns Dir object.
 

$dir.read() : Maybe[String]
Read a entitity from $dir.
If it reached end of directory, it returns undef.
 

$dir.close() : undef
Close directory.
Normally, you don't need to close directory by hand since the destructor closes directory automtically.
 

Dir.mkdir(Str $name[, Int $mode]) : Undef
Create directory named $name. Default mode value is 0777.
This method may throws ErrnoException if it's failed.
 

Dir.rmdir(Str $name) : Undef
Remove $name directory.
This method may throws ErrnoException if it's failed.
 

$dir.DESTROY() : undef
This is a destructor, closes directory automatically.
 

Dir.__iter__()
Directory object can use as iterator.
Example:
  for (my $e in dir('t')) { ... }
 

Dir::Iterator.__next__()
Get a next iteration object from iterator.
 

