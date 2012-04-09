class Object
=============

Object is a base class of all classes.
It's known as UNIVERSAL package in Perl5.
 

$object.tora() : String
convert object to tora source.
 

$object.meta() : MetaClass
get a meta class.
 

$object.isa($target) : Boolean
This method returns true if $object is-a $target, false otherwise.
 

$object.bless(Any $data) : Object
Create new instance with $data.
 

