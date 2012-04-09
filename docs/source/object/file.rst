class File
===========

The file class.
 

File.open(Str $fname) : FileHandle
File.open(Str $fname, Str $mode) : FileHandle
Open the file named $fname by $mode.
Str $mode: file opening mode. You can specify the option as following:
  "w": writing mode
  "r": reading mode
  "a": appending mode
File.open passes $mode to fopen(3).
 

$file.slurp() : String
Read all file content and return it in string.
 

$file.close() : Undef
Close a file.
 

$file.write(Str $str); : Undef
write $string to a file.
 

$file.flush();
flush a writing buffer in stdio.
 

$file.fileno();
Get a file number in file handler.
 

$file.getc() : Maybe[Str]
Get a character from $file.
If EOF reached, returns undef.
 

$file.seek(Int offset, Int $whence) : Undef
Seek a file pointer position from $file.
$whence must be one of File.SEEK_CUR, File.SEEK_END, File.SEEK_SET.
maybe throw ErrnoException.
 

$file.tell() : Int
Get a file pointer position from $file.
maybe throw ErrnoException.
 

$file.sync() : Undef
Syncs buffers and file.
On win32, this method is not implementede yet.
 

