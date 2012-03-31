class DynaLoader
=================


This is a dynamic extension library loader for tora.
This is a low level library.
You don't need to use this module directly in normal case. You can use XSLoader instead.
 

DynaLoader.load(Str $filename, Str $endpoint) : Undef
Load a extension library from file $filename.
This method loads $filename by dlopen(3) and find symbol named $endpoint, and call it.
 

