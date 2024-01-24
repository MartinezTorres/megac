/* Let's try to minimize wheel inventing. We'll reuse what we can. Be as close as possible to C */

// Numeric constants:
// Native numeric constants are all unsigned and use fewest number of bytes necessary to represent the value.
// 

// Native types:
// intX -> signed integers of X bits.
// uintX -> unsigned integers of X bits.
// floatX -> floating point value of X bits.
// 

// Structures:
// defined by name struct following a span.
// structures fulfill the role of namesapces.
// stuctures are always anonymous, but can be assigned a name using def. The name must start with a capital and be camel cased.
// 
// Content:
// Structures can hold members. 
// Structures can hold functions.
// Structures can be given traits.
// packing is always dense at the bit level.

// Traits, define properties of a type:
// defined by "trait" and a following span.
// traits are always anonymous, abeit this makes little sense.  Can be assigned a name using def. 
// Traits contain only fuctions.
// Machine specific properties can ve conveyed using traits (e.g., forcing a memory address).
// Traits can be added to a type definition, but also to a variable definition.

// Structures and Traits can be given parameters. This is the templating mechanism. 
// This is like a function but known at compile time, so it uses normal parentheses.
// It uses direct token replacement mechanism (hence useful as macro also).

// include simply adds the content of a file at the desired location, not the best option
// using loads a module

import system;
// Equivalent: alias Vector := Stdlib.Vector, String := Stdlib.String;
// Also possible: from Stdlib use everything;


// Hello World!

def type Vector := struct<#T> {
	
	var uint8 sz;
	var ptr<#T> data;
	
	operator [] (uint8 idx) -> (ref<#T> { return (data + idx).ref; }
};

def type String := Vector<uint8>;

def fn main (int32 argc, ptr<ptr<uint8>> argv ) -> (int32) @extern {
	
	system.terminal.out << "Hello World!" << system.terminal.endl;
	
}



