########################################################################
	// It follows a format similar to BISON with the following exceptions. 
	// A term between [square_brackets] is optional.
	// Symbols between "double quotes" can be reduced if only have one child.
	// Terms between "double quotes" won't be stored as children.
	// Based on: https://www.lysator.liu.se/c/ANSI-C-grammar-y.html

	;

RESERVED_KEYWORDS : int | goto | alignas | alignof  | and | and_eq | asm | atomic_cancel  | atomic_commit  | atomic_noexcept  | auto     | bitand | bitor | bool | break | case | catch | char | char8_t  | char16_t  | char32_t  | class  | compl | concept  | const | consteval  | constexpr  | constinit  | const_cast | continue | co_await  | co_return  | co_yield  | decltype  | default  | delete  | do | double | dynamic_cast | else | enum  | explicit | export   | extern  | false | float | for  | friend | goto | if   | inline  | int | long | mutable  | namespace | new | noexcept  | not | not_eq | nullptr  | operator  | or | or_eq | private  | protected | public | reflexpr  | register  | reinterpret_cast | requires  | return | short | signed | sizeof  | static | static_assert  | static_cast | struct  | switch | synchronized  | template | this  | thread_local  | throw | true | try | typedef | typeid | typename | union | unsigned | using  | virtual | void | volatile | wchar_t | while | xor | xor_eq ;

ERASED : ERASED ;

IDENTIFIER : IDENTIFIER ;
CONSTANT : CONSTANT ;
STRING_LITERAL : STRING_LITERAL ;

namespace_identifier
	: IDENTIFIER ::
	| namespace_identifier IDENTIFIER :: 
	;

"primary_expression"
	: [namespace_identifier] IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| "(" expression ")"
	;

"postfix_expression"
	: primary_expression
	| postfix_expression [ expression ]
	| postfix_expression ( )
	| postfix_expression ( argument_expression_list )
	| postfix_expression . IDENTIFIER
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list "," assignment_expression
	;

"unary_expression"
	: postfix_expression
	| ++ unary_expression
	| -- unary_expression
	| unary_operator cast_expression
	| sizeof unary_expression
	| sizeof ( type_name )
	;

unary_operator : !& | !+ | !- | !~ | !! ;

"cast_expression"
	: unary_expression
	| ( type_name ) cast_expression
	;

"multiplicative_expression"
	: cast_expression
	| multiplicative_expression !* cast_expression
	| multiplicative_expression !/ cast_expression
	| multiplicative_expression !% cast_expression
	;

"additive_expression"
	: multiplicative_expression
	| additive_expression !+ multiplicative_expression
	| additive_expression !- multiplicative_expression
	;

"shift_expression"
	: additive_expression
	| shift_expression !<< additive_expression
	| shift_expression !>> additive_expression
	;

"relational_expression"
	: shift_expression
	| relational_expression !< shift_expression
	| relational_expression !> shift_expression
	| relational_expression !<= shift_expression
	| relational_expression !>= shift_expression
	;

"equality_expression"
	: relational_expression
	| equality_expression !== relational_expression
	| equality_expression !!= relational_expression
	;

"and_expression"
	: equality_expression
	| and_expression !& equality_expression
	;

"exclusive_or_expression"
	: and_expression
	| exclusive_or_expression !^ and_expression
	;

"inclusive_or_expression"
	: exclusive_or_expression
	| inclusive_or_expression !| exclusive_or_expression
	;

"logical_and_expression"
	: inclusive_or_expression
	| logical_and_expression !and inclusive_or_expression
	;

"logical_or_expression"
	: logical_and_expression
	| logical_or_expression !or logical_and_expression
	;

"conditional_expression"
	: logical_or_expression
	| logical_or_expression ? expression : conditional_expression
	;

"assignment_expression"
	: conditional_expression
	| unary_expression !assignment_operator assignment_expression
	;

"assignment_operator" : != | >>= | <<= | += | -= | *= | /= | %= | &= | ^= | |= ;

"expression"
	: assignment_expression
	| expression "," assignment_expression
	;
	
########################################################################
# DECLARATIONS ;	
	
"declaration"
	: ";"
	| type_name init_declarator_list ";"
	| !typedef type_name IDENTIFIER ";"
	| "#" !include STRING_LITERAL
	;

"init_declarator_list"
	: IDENTIFIER
	| init=: IDENTIFIER "=" initializer
	| init_declarator_list "," IDENTIFIER
	| init_declarator_list "," IDENTIFIER = initializer
	;

"initializer"
	: assignment_expression
	| "{" initializer_list [","] "}"
	;

"initializer_list"
	: initializer
	| initializer_list "," initializer
	;

type_name
	: [namespace_identifier] IDENTIFIER
	| void
	| uint8
	| int8
	| uint16
	| int16
	| array: type_name "[" [CONSTANT] "]"
	| struct_type: "struct" translation_unit_scoped
	| union_type: "union2 translation_unit_scoped
	| function_type: "function" "[" type_name parameter_list_scoped "]"
	| bit_field: type_name : CONSTANT
	;

########################################################################
# STATEMENT ;

statement_list_scoped : "{" [statement_list] "}" ;

"statement_list"
	: statement
	| statement_list statement
	;

statement
	: "{" [statement_list] "}"
	| ";"
	| declaration
	| expression ";"
	| if ( expression ) statement
	| if ( expression ) statement else statement
	| while ( expression ) statement
	| do statement while ( expression ) ";"
	| for ( statement expression ';' expression ) statement
	| continue ";"
	| break ";"
	| return ";"
	| return expression ";"
	;

########################################################################
# FUNCTION DEFINITION ;

parameter_list_scoped : "(" [parameter_list] ")" ;

parameter_list
	: type_name
	| type_name IDENTIFIER
	| parameter_list "," type_name
	| parameter_list "," type_name IDENTIFIER
	;

function_name : [namespace_identifier] IDENTIFIER ;

function_definition
	: type_name function_name parameter_list_scoped statement_list_scoped
	;

########################################################################
# TRANSLATION UNIT ;

translation_unit_scoped : "{" [translation_unit] "}" ;

"translation_unit"
	: function_definition
	| declaration
	| translation_unit function_definition
	| translation_unit declaration
	;

"start" : translation_unit ;
