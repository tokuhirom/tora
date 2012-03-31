class RE2::Regexp::Matched
===========================

This is a regexp matched object for RE2.
You would get a instance of this class in String#match method.
 

$matched.regexp() : Regexp
Get a source regular expression object.
 

$matched.to_array() : Array[String]
Convert $matched object to array of strings.
 

$matched.__getitem__(Int $i)    : Maybe[String]
$matched.__getitem__(Str $name) : Maybe[String]
Get a matched string piece by index $i.
$matched.__getitem__(Str $name) returns named capture string, but it is not implemented yet.
 

