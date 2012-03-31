class String
=============

The string class.
 

$string.length() : Int
Get a String length.
 

$string.match(Regexp $pattern) : Maybe[RE2::Regexp::Matched]
$string.match(String $pattern) : Maybe[RE2::Regexp::Matched]
$string match with $pattern. If it does not matched, returns undefinied value.
If it's matched, returns RE2::Regexp::Matched object.
 

$string.replace(Regexp $pattern, String $replacer) : Str
$string.replace(String $pattern, String $replacer) : Str
Replace string parts by regexp or string. It returns replaced string.
This method does not rewrite original $string.
 

$string.substr(Int $start)              : Str
$string.substr(Int $start, Int $length) : Str
It returns substring from $string.
Example:
  "foobar".substr(3) # => "bar"
  "foobar".substr(3,2) # => "ba"
Get a substring from string.
 

$string.scan(Regexp $pattern) : Array[String]
Scan the strings by regular expression.
 

$string.split(Regexp $pattern) : Array[String]
Examples:
  "foo".split(//) # => qw(f o o)
  "a\nb\nc".split(/\n/) # => qw(a b c)
Split a string by regexp.
 

