class MetaClass
================

This is a metaclass. Meta class is class of class.
You can get a meta class for $foo from $foo.meta().

Note. API is compatible with Moose in Perl5, preferably.
 

$meta.has_method(Str $name) : Boolean
Returns a boolean indicating whether or not the class defines the named method.
It does not include methods inherited from parent classes.
 

$meta.get_method_list() : Array[Str]
Get a method list defined in package.
 

$meta.name() : String
Get a name of class.
 

$meta.superclass() : String
This method returns superclass in string.
If the class does not have a superclass, it returns undef.
 

