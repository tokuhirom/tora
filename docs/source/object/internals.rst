class Internals
================

This is a utility functions for debugging tora itself.
APIs may change without notice.
 

Internals.stack_size() : Int

Returns the number of items in stack.
 

Internals.dump_stack() : Undef
Shows dump of stacks to stdout.
(Format may changes without notice.)
 

Internals.dump(Any $value); : Undef
dump SV to stdout.
 

Internals.dump_symbol_table() : Undef
Dump symbol table to stdout.
 

