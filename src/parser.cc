#include <parser.h>

#include <log.h>

#include <optional>
#include <set>

std::string SyntaxTree::to_string(std::string prefix) const {
	
	std::ostringstream oss;
	oss << prefix << "- " << symbol << std::endl;
	for (auto &c : children) {
		oss << prefix + "  |" << std::endl;
		oss << c.to_string(prefix + (&c == &children.back()?"   ":"  |"));
	}
	return oss.str();
}

struct Grammar {
	
	
	// We store the grammar inside an struct to be able to hide it. It follows BISON format.
	// Based on: https://www.lysator.liu.se/c/ANSI-C-grammar-y.html
	struct GrammarDescription : public std::string {
		
		GrammarDescription() : std::string (
R"GRAMMAR(

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start translation_unit
%%

IDENTIFIER : "IDENTIFIER" ;
CONSTANT : "CONSTANT" ;
STRING_LITERAL : "STRING_LITERAL" ;

ELLIPSIS : "..." ;	
RIGHT_ASSIGN : ">>=" ;	
LEFT_ASSIGN : "<<=" ;	
ADD_ASSIGN : "+=" ;	
SUB_ASSIGN : "-=" ;	
MUL_ASSIGN : "*=" ;	
DIV_ASSIGN : "/=" ;	
MOD_ASSIGN : "%=" ;	
AND_ASSIGN : "&=" ;	
XOR_ASSIGN : "^=" ;	
OR_ASSIGN : "|=" ;	
RIGHT_OP : ">>" ;	
LEFT_OP : "<<" ;	
INC_OP : "++" ;	
DEC_OP : "--" ;	
PTR_OP : "->" ;	
AND_OP : "&&" | "and" ;	
OR_OP : "||" | "or" ;	
LE_OP : "<=" ;	
GE_OP : ">=" ;	
EQ_OP : "==" ;	
NE_OP : "!=" | "xor" ;

AUTO :      "auto" ;  
BREAK :     "break" ;  
CASE :      "case" ;  
CHAR :      "char" ;  
CONST :     "const" ;  
CONTINUE :  "continue" ; 
DEFAULT :   "default" ; 
DO :        "do" ;  
DOUBLE :    "double" ; 
ELSE :      "else" ;  
ENUM :      "enum" ;  
EXTERN :    "extern" ; 
FLOAT :     "float" ;  
FOR :       "for" ;  
GOTO :      "goto" ;  
IF :        "if" ;  
INT :       "int" ;  
LONG :      "long" ;  
REGISTER :  "register" ; 
RETURN :    "return" ; 
SHORT :     "short" ;  
SIGNED :    "signed" ; 
SIZEOF :    "sizeof" ; 
STATIC :    "static" ; 
STRUCT :    "struct" ; 
SWITCH :    "switch" ; 
TYPEDEF :   "typedef" ; 
UNION :     "union" ;  
UNSIGNED :  "unsigned" ; 
VOID :      "void" ;  
VOLATILE :  "volatile" ; 
WHILE :     "while" ;  

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression
	: assignment_expression
	| expression ',' assignment_expression
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
	;

declaration_specifiers
	: storage_class_specifier
	| storage_class_specifier declaration_specifiers
	| type_specifier
	| type_specifier declaration_specifiers
	| type_qualifier
	| type_qualifier declaration_specifiers
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator
	;

init_declarator
	: declarator
	| declarator '=' initializer
	;

storage_class_specifier
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;

type_qualifier
	: CONST
	| VOLATILE
	;

declarator
	: pointer direct_declarator
	| direct_declarator
	;

direct_declarator
	: IDENTIFIER
	| '(' declarator ')'
	| direct_declarator '[' constant_expression ']'
	| direct_declarator '[' ']'
	| direct_declarator '(' parameter_type_list ')'
	| direct_declarator '(' identifier_list ')'
	| direct_declarator '(' ')'
	;

pointer
	: '*'
	| '*' type_qualifier_list
	| '*' pointer
	| '*' type_qualifier_list pointer
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expression ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	;

initializer_list
	: initializer
	| initializer_list ',' initializer
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{' statement_list '}'
	| '{' declaration_list '}'
	| '{' declaration_list statement_list '}'
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: ';'
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
	| declaration_specifiers declarator compound_statement
	| declarator declaration_list compound_statement
	| declarator compound_statement
	;

)GRAMMAR"		
		
		) {}

	} grammar_string;
	
	// A grammar symbol can be build from one of many possible recipes.
	struct Symbol {
		
		// Symbol name
		std::string name;
		
		// Each symbol recipe is a list of different components
		std::vector<std::vector<std::string>> recipes;
	};
	
	std::map<std::string, Symbol> symbols;
	
	Grammar() {
		
		std::istringstream iss(grammar_string);
		std::string s;
		while (iss >> s and s!="%%"); 

		while (iss >> s) {
			
			
			Symbol &symbol = symbols[s];
			symbol.name = s;
			
			iss >> s;
			Assert(s==":") << ": expected, got " << s;
			
			std::vector<std::string> recipe;
			while (iss >> s) {
				
				if (s=="|" or s==";") { // recipe is finished.
					
					Assert(not recipe.empty()) << " found an empty recipe";
					
					symbol.recipes.push_back(recipe);
					recipe.clear();
					if (s==";") break;
					
				} else {
					
					Assert(not s.empty()) << " broken grammar";
					
					if (s[0]=='\'' or s[0]=='"') {
						
						recipe.push_back( s.substr(1,s.size()-2) );
					
					} else {
						
						recipe.push_back(s);
					} 
				}
			}
		}
	}
};

static Grammar grammar;

struct ParseDebug {
	SyntaxTree::TI last_error_token;
	std::vector<std::string> expected_targets;
};

static std::optional<SyntaxTree> parse(SyntaxTree::TI begin, SyntaxTree::TI end, std::string target, ParseDebug &debug) {
	
	// tackle leaf nodes: which can be a keyword, an identifier, a numeric constant, or a string literal.
	{
		
		auto expect = [&](bool condition) -> std::optional<SyntaxTree> {
			if (not condition or begin==end) {
				if (debug.last_error_token<begin) {
					debug.last_error_token = begin;
					debug.expected_targets.clear();
				}
				if (debug.last_error_token==begin) {
					debug.expected_targets.push_back(target);
				}
				return std::nullopt;
			}
			return SyntaxTree(begin, target);
		};

		if (grammar.symbols.count(target) == 0) 
			return expect( ( begin->type != Token::STRING_LITERAL ) and (begin->literal == target) );

		// Also there are some leaf nodes harcoded in the symbol table
		if ( target == "IDENTIFIER" ) 
			return expect(begin->type == Token::IDENTIFIER);

		if ( target == "CONSTANT" ) 
			return expect(begin->type == Token::NUMERIC);
		
		if ( target == "STRING_LITERAL" ) 
			return expect(begin->type == Token::STRING_LITERAL);
	} 
	
	// for each target symbol, there may be several recipes possible, we will accept only the longest.
	std::optional<SyntaxTree> best_recipe_opt;

	// for each symbol, we tacke first the recipes that aren't front-recursive
	for (auto &recipe : grammar.symbols[target].recipes) {
		
		// skip front-recursive recipes.
		if (recipe.front() == target) continue;
		
		SyntaxTree recipe_tree(begin, target);
		
		auto it = begin;

		bool is_valid = true;
		for (auto &component : recipe) {
			
			std::optional<SyntaxTree> component_tree_opt = parse(it, end, component, debug);
			if (not component_tree_opt.has_value()) { 
				
				is_valid = false;
				break;
			}
			
			auto &component_tree = component_tree_opt.value();
			
			recipe_tree.children.push_back( component_tree );
			recipe_tree.end = component_tree.end;
			it = component_tree.end;
		}
		
		if (not is_valid) continue;
		
		if (not best_recipe_opt.has_value()) {
		
			best_recipe_opt = recipe_tree;
			continue;
		}
		
		if (best_recipe_opt.value().end < recipe_tree.end) {
			
			best_recipe_opt = recipe_tree;
			continue;
		}
	}
	
	// for each symbol, we now tacke the recipes that are front-recursive
	if (best_recipe_opt.has_value()) {
		
		bool expanded = true;
		
		while (expanded) {
			
			expanded = false;
			SyntaxTree previous_recipe = best_recipe_opt.value();
			
			for (auto &recipe : grammar.symbols[target].recipes) {
				
				// skip non front-recursive recipes.
				if (recipe.front() != target) continue;
				
				SyntaxTree recipe_tree(begin, target);
				
				auto it = begin;
				
				bool is_valid = true;

				for (auto &component : recipe) {
					
					if (it==begin) {
						
						recipe_tree.children.push_back( previous_recipe );
						recipe_tree.end = previous_recipe.end;
						it = previous_recipe.end;
						continue;
					}
					
					std::optional<SyntaxTree> component_tree_opt = parse(it, end, component, debug);
					if (not component_tree_opt.has_value()) {
						is_valid = false;
						break;
					}
					
					auto &component_tree = component_tree_opt.value();
					
					recipe_tree.children.push_back( component_tree );
					recipe_tree.end = component_tree.end;
					it = component_tree.end;
				}
				
				if (not is_valid) continue;
				
				if (best_recipe_opt.value().end < recipe_tree.end) {
			
					best_recipe_opt = recipe_tree;
					expanded = true;
				}
			}
		}
	}
	
	return best_recipe_opt;
}

SyntaxTree::SyntaxTree(TI _begin, std::string _symbol) : begin(_begin), end(++ _begin), symbol(_symbol) {}


SyntaxTree::SyntaxTree(SourceFile &file, std::string root) {

	auto &tokens = tokenize(file);
	
	ParseDebug debug; 
	debug.last_error_token = tokens.begin();
	
	auto ast_opt = parse(tokens.begin(), tokens.end(), root, debug);
	
	if (not ast_opt.has_value()) {

		std::set<std::string> unique_targets;
		for (auto &target : debug.expected_targets) 
			unique_targets.insert(target);
		
		std::ostringstream oss;
		for (auto &target : unique_targets) {
			if (&target != &*unique_targets.begin()) oss << " ";
			oss << "'" << target << "'";
		}
		
		if (debug.last_error_token != tokens.end()) {
			auto &token = debug.last_error_token;
			Log(ERROR) << "Parser failed in line " << token->begin_ptr.get_line() << ". Expecting any of " << oss.str() << " but found " << token->to_string() << ".\n" << token->to_line_string();
		} else {
			Log(ERROR) << "Parser failed. Expecting any of " << oss.str() << " but the file ended";
		}
	}

	*this = ast_opt.value();
}
	
