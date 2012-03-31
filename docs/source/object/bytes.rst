class Bytes
============

Bytes class is character set independent byte string class.
 

$bytes.length() : Int
Return the caller code object.
 

$bytes.substr(Int $start)              : Str
$bytes.substr(Int $start, Int $length) : Str
It returns substring from $bytes.
Example:
  b"foobar".substr(3) # => "bar"
  b"foobar".substr(3,2) # => "ba"
Get a substring from bytes.
 

