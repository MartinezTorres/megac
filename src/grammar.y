// ########################################################################
// #  GRAMMAR SPECIFICATION FOR MEGAC
// # 
// # It follows a format similar to BISON with the following keywords.
// # 
// # %opt <term> : <term> may be ommited
// # %weak <symbol>: if <symbol> has only one child, it can be reduced while parsing.
// # %label <name>: if the recipe is accepted, the tree will be labeled with <name> 
// # %root <term> : <term> will replace the symbol at the root of the tree 
// # 
// # Based on: https://www.lysator.liu.se/c/ANSI-C-grammar-y.html
// # 

// To avoid confusion, we won't allow identifiers to use the name of a reserved keyword of C++
%reserved_keywords int goto alignas alignof and and_eq asm atomic_cancel atomic_commit atomic_noexcept auto bitand bitor bool break case catch char char8_t char16_t char32_t class compl concept  const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default delete do double dynamic_cast else enum explicit export extern false float for friend function goto if inline int long mutable namespace new noexcept not not_eq nullptr operator or or_eq private protected public reflexpr register reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch synchronized template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while xor xor_eq uint8 uint16 int8 int16;

// Key Tokens
%token IDENTIFIER CONSTANT STRING_LITERAL ;

%%

// ########################################################################
// # EXPRESSIONS 

namespaced_identifier
	: IDENTIFIER	
	| namespaced_identifier '::' IDENTIFIER
	;

%weak
primary_expression 
	: namespaced_identifier
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

%weak
postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| %label 'function_call' postfix_expression '(' %opt argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

%weak
unary_expression
	: postfix_expression
	| '++' unary_expression
	| '--' unary_expression
	| unary_operator cast_expression
	| 'sizeof' unary_expression
	| 'sizeof' '(' type_name ')'
	;

unary_operator : '&' | '+' | '-' | '~' | '!' ;

%weak
cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

%weak
multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

%weak
additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

%weak
shift_expression
	: additive_expression
	| shift_expression '<<' additive_expression
	| shift_expression '>>' additive_expression
	;

%weak
relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	;

%weak
equality_expression
	: relational_expression
	| equality_expression '==' relational_expression
	| equality_expression '!=' relational_expression
	;

%weak
and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

%weak
exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

%weak
inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

%weak
logical_and_expression
	: inclusive_or_expression
	| logical_and_expression 'and' inclusive_or_expression
	;

%weak
logical_or_expression
	: logical_and_expression
	| logical_or_expression 'or' logical_and_expression
	;

%weak
conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

%weak
assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
	;

%weak
assignment_operator : '!=' | '>>=' | '<<=' | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '^=' | '|=' ;


%weak
expression
	: assignment_expression
	| expression ',' assignment_expression
	;
	
// ########################################################################
// # DECLARATIONS 
	

init_declaration 
	: %root IDENTIFIER
	| IDENTIFIER %root '=' initializer 
	;

init_declarator_list
	: init_declaration
	| init_declarator_list ',' init_declaration
	;

%weak
initializer
	: assignment_expression
	| '{' initializer_list %opt ',' '}'
	;

%weak
initializer_list
	: initializer
	| initializer_list ',' initializer
	;

type_name
	: namespaced_identifier
	| %keep 'void'
	| %keep 'uint8'
	| %keep 'int8'
	| %keep 'uint16'
	| %keep 'int16'
	| %label 'array' type_name '[' %opt CONSTANT ']'
	| %label 'struct' 'struct' '{' %opt translation_unit '}' 
	| %label 'union' 'union' '{' %opt translation_unit '}' 
	| %label 'function_type' %root 'function' '[' type_name parameter_list_scoped ']'
	| %label 'bit_field' type_name ':' CONSTANT
	| type_name attributes 
	;

// ########################################################################
// # ATTRIBUTES

attribute
	: %root 'asm'
	| %keep 'register' %root '=' IDENTIFIER
	| IDENTIFIER %root '=' CONSTANT
	| %root IDENTIFIER
	;

attribute_list
	: attribute
	| attribute_list ',' attribute
	;

%weak
attributes
	: '<' %opt attribute_list '>'
	;

// ########################################################################
// # STATEMENT 

%weak
statement
	: '{' %opt translation_unit '}'
	| ';'
	| function_definition
	| expression ';'
	| %root 'if' '(' expression ')' statement
	| %root 'if' '(' expression ')' statement 'else' statement
	| %root 'while' '(' expression ')' statement
	| %root 'do' statement 'while' '(' expression ')' ';'
	| %root 'for' '(' statement expression ';' expression ')' statement
	| %root 'for' '(' 'auto' IDENTIFIER ':' expression ')' statement
	| %root 'continue' ';'
	| %root 'break' ';'
	| %root 'return' ';'
	| %root 'return' expression ';'
	| %label 'type_declaration' type_name init_declarator_list ';'
	| %root 'typedef' type_name IDENTIFIER ';'
	| '#' %root 'include' STRING_LITERAL
	| %root 'namespace' IDENTIFIER '{' %opt translation_unit '}'
	;

// ########################################################################
// # FUNCTION DEFINITION 

parameter_list_scoped : '(' %opt parameter_list ')' ;

parameter_list
	: type_name
	| type_name IDENTIFIER
	| parameter_list ',' type_name
	| parameter_list ',' type_name IDENTIFIER
	;

function_name : IDENTIFIER ;

function_body :  '{' %opt translation_unit '}' ;

function_definition	: type_name function_name parameter_list_scoped function_body ;

// ########################################################################
// # TRANSLATION UNIT 

translation_unit
	: statement
	| translation_unit statement
	;

%weak
start 
	: translation_unit 
	;
