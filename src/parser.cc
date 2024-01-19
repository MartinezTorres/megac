#include <parser.h>

#include <log.h>

#include <optional>
#include <set>

std::string SyntaxTree::to_string(std::string prefix) const {
	
	std::ostringstream oss;
	if (symbol == "IDENTIFIER") { oss << prefix << "- " << symbol << " " << first->to_string() << std::endl;
	} else { oss << prefix << "- " << symbol << std::endl;
	}
	for (auto &c : children) {
		oss << prefix + "  |" << std::endl;
		oss << c.to_string(prefix + (&c == &children.back()?"   ":"  |"));
	}
	return oss.str();
}

struct Grammar {
	
	
	// We store the grammar inside an struct to be able to fold it in the gui. 
	// It follows a format similar to BISON with the following exceptions. 
	// A term between [square_brackets] is optional.
	// Symbols between "double quotes" can be reduced if only have one child.
	// Terms between "double quotes" won't be stored as children.
	// Based on: https://www.lysator.liu.se/c/ANSI-C-grammar-y.html
	struct GrammarDescription : public std::string {
		GrammarDescription() : std::string (
R"GRAMMAR(
########################################################################
# EXPRESSIONS ;

RESERVED_KEYWORDS : int | goto | alignas | alignof  | and | and_eq | asm | atomic_cancel  | atomic_commit  | atomic_noexcept  | auto     | bitand | bitor | bool | break | case | catch | char | char8_t  | char16_t  | char32_t  | class  | compl | concept  | const | consteval  | constexpr  | constinit  | const_cast | continue | co_await  | co_return  | co_yield  | decltype  | default  | delete  | do | double | dynamic_cast | else | enum  | explicit | export   | extern  | false | float | for  | friend | goto | if   | inline  | int | long | mutable  | namespace | new | noexcept  | not | not_eq | nullptr  | operator  | or | or_eq | private  | protected | public | reflexpr  | register  | reinterpret_cast | requires  | return | short | signed | sizeof  | static | static_assert  | static_cast | struct  | switch | synchronized  | template | this  | thread_local  | throw | true | try | typedef | typeid | typename | union | unsigned | using  | virtual | void | volatile | wchar_t | while | xor | xor_eq ;

ERASED : ERASED ;

IDENTIFIER : IDENTIFIER ;
CONSTANT : CONSTANT ;
STRING_LITERAL : STRING_LITERAL ;

"primary_expression"
	: IDENTIFIER
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
	: IDENTIFIER
	| void
	| uint8
	| int8
	| uint16
	| int16
	| array: type_name "[" [CONSTANT] "]"
	| !struct "{" [translation_unit] "}"
	| !union "{" [translation_unit] "}"
	| !function [ [type_name] ( [parameter_list] ) ]
	| bit_field: type_name : CONSTANT
	;

########################################################################
# STATEMENT ;

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

parameter_list
	: type_name
	| type_name IDENTIFIER
	| parameter_list "," type_name
	| parameter_list "," type_name IDENTIFIER
	;

function_definition
	: type_name IDENTIFIER "(" [parameter_list] ")" "{" [statement_list] "}"
	;

########################################################################
# TRANSLATION UNIT ;

translation_unit
	: function_definition
	| declaration
	| translation_unit function_definition
	| translation_unit declaration
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
	std::set<std::string> keywords, reducible_symbols;
	std::string subsymbol_name;
	
	Grammar() {
		
		std::istringstream iss(grammar_string);
		std::string s;

		// Let's make a list of found symbols first
		while (iss >> s) {
			
			if (s.front()=='#') { while (iss >> s) if (s == ";") break; iss >> s; }
			
			symbols[s];
			
			while (iss >> s) if (s == ";") break;
		}

		iss = std::istringstream(grammar_string);
		while (iss >> s) {
			
			if (s.front()=='#') { while (iss >> s) if (s == ";") break; iss >> s; }
			
			if (s.front() == '"') {
				s = s.substr(1,s.size()-2);
				reducible_symbols.insert(s);
			}

			Symbol &symbol = symbols[s];
			symbol.name = s;
			
			iss >> s;
			Assert(s==":") << ": expected, got " << s;
			
			std::vector<std::vector<std::string>> recipes;
			while (iss >> s) {
				
				if (s=="|" or s==";") { // recipe is finished.
					
					if (subsymbol_name.empty()) {
						
						for (auto &recipe : recipes) {
							Assert(not recipe.empty()) << " found an empty recipe in symbol " << symbol.name;
							symbol.recipes.push_back(recipe);
						}
						recipes.clear();
					} else {
						
						Log(ERROR_NOTHROW) << "Subsymbol: " << subsymbol_name;
						symbol.recipes.emplace_back(1,subsymbol_name);
						
						Symbol &subsymbol = symbols[subsymbol_name];
						subsymbol.name = subsymbol_name;

						for (auto &recipe : recipes) {
							Assert(not recipe.empty()) << " found an empty recipe in symbol " << subsymbol_name;
							Log(ERROR_NOTHROW) << "Subsymbol: " << subsymbol_name << " recipe: " << ([&](){std::ostringstream oss; for (auto &v:recipe) oss<<v<<" "; return oss.str();}());
							subsymbol.recipes.push_back(recipe);
						}
						recipes.clear();
												
						subsymbol_name.clear();
					}
					if (s==";") break;
					
				} else if (s.size()>1 and s.back() == ':') {
				
					s.pop_back();
					subsymbol_name = s;
					symbols[s];
					 
				} else {
					
					Assert(not s.empty()) << " broken grammar";

					bool optional = false;

					if (s.front() == '\'') {
						s = s.substr(1,s.size()-2);
					}

					if (s.front() == '[' and s.back() == ']') {
						s = s.substr(1,s.size()-2);
						optional = true;
					}

					
					
					if (recipes.empty()) recipes.emplace_back();

					if (optional) {
						
						auto new_recipes = recipes;
						for (auto &recipe : new_recipes) {

							recipe.push_back(s);
							recipes.push_back(recipe);
						}
					} else {
						for (auto &recipe : recipes) {
							recipe.push_back(s);
						}
					}

					if (symbols.count(s) == 0)
						keywords.insert(s);
				}
			}
		}
		Log(DEBUG) << " Grammar found " << keywords.size() << " keywords.";
		Log(EXTRA) << [&](){ std::ostringstream oss; for (auto &keyword : keywords) oss << keyword << " "; return oss.str(); }(); 
	}
};

static Grammar grammar;

struct ParseDebug {
	SyntaxTree::TI last_error_token;
	std::vector<std::string> expected_targets;
};

static std::vector<SyntaxTree> parse(SyntaxTree::TI token_it, SyntaxTree::TI last_token, std::string target, ParseDebug &debug, std::vector<std::string> parent_targets_without_consuming_tokens = std::vector<std::string>()) {

	// Quick workaround, kill deep chains
	//if (parent_targets_without_consuming_tokens.size()>50) 
	//	return std::vector<SyntaxTree>();
	
	// This filters out non-trivially front-recursive recipes. I haven't slept in a week while trying and failing to find a general solution for this problem. Randomly, some kind of programming deity just put me this idea on my mind. It seems to work. I have no idea why.
	if (parent_targets_without_consuming_tokens.size()>2) {
		size_t N = parent_targets_without_consuming_tokens.size();
		bool parent_is_on_the_list = false;
		for (size_t n = 0; n < N-2; n++)
			if (parent_targets_without_consuming_tokens[n] == parent_targets_without_consuming_tokens[N-2])
				parent_is_on_the_list = true;
				
		if (parent_is_on_the_list) {
			for (size_t n = 0; n < N-1; n++)
				if (parent_targets_without_consuming_tokens[n] == parent_targets_without_consuming_tokens[N-1])
					return std::vector<SyntaxTree>();
		}
	}
	


	// tackle leaf nodes: which can be a keyword, an identifier, a numeric constant, or a string literal.
	{
		auto expect = [&](bool condition) -> std::vector<SyntaxTree> {
			if (not condition or token_it == last_token) {
				if (debug.last_error_token < token_it) {
					debug.last_error_token = token_it;
					debug.expected_targets.clear();
				}
				if (debug.last_error_token == token_it) {
					debug.expected_targets.push_back(target);
				}
				return std::vector<SyntaxTree>();
			}
			if (target.front()=='"') return std::vector<SyntaxTree>(1, SyntaxTree(token_it, "ERASED"));
			return std::vector<SyntaxTree>(1, SyntaxTree(token_it, target));
		};

		if (grammar.symbols.count(target) == 0 and target.front() == '"') 
			return expect( (token_it->type != Token::STRING_LITERAL) and ('"' + token_it->literal + '"' == target) );

		if (grammar.symbols.count(target) == 0 and target.front() == '!') 
			return expect( (token_it->type != Token::STRING_LITERAL) and ('!' + token_it->literal == target) );

		if (grammar.symbols.count(target) == 0) 
			return expect( (token_it->type != Token::STRING_LITERAL) and (token_it->literal == target) );

		// Also there are some leaf nodes harcoded in the symbol table
		if ( target == "IDENTIFIER" ) 
			return expect( (token_it->type == Token::IDENTIFIER) and (grammar.keywords.count(token_it->literal) == 0) );

		if ( target == "CONSTANT" ) 
			return expect(token_it->type == Token::NUMERIC);
		
		if ( target == "STRING_LITERAL" ) 
			return expect(token_it->type == Token::STRING_LITERAL);
			
	} 

	
	// for each target symbol, there may be several recipes possible.
	std::vector<SyntaxTree> all_ast;

	// register our current target.
	parent_targets_without_consuming_tokens.push_back(target);

	// for each symbol, we tackle first the recipes that aren't front-recursive
	for (auto &recipe : grammar.symbols[target].recipes) {

		// skip pure front-recursive recipes.
		{
			bool is_front_recursive_recipe = (recipe.front() == target);
			if (is_front_recursive_recipe) continue;
		}

		std::vector<SyntaxTree> ast;
		ast.emplace_back(token_it, target);
		ast.back().last = token_it;

		for (auto component : recipe) {

			std::vector<SyntaxTree> tentative_ast;
			
			for (auto &a : ast) {

				
				bool is_first = ( a.last == token_it );
				
				auto &&t = (is_first?parent_targets_without_consuming_tokens:std::vector<std::string>()); 

				for (auto &c : parse(a.last, last_token, component, debug, t)) {
				
					auto a2 = a;
					a2.last = c.last;
					if (component.front() == '!' and component.size()>1) {
						a2.symbol = c.symbol.substr(1);
					} else if (c.symbol != "ERASED") {
						a2.children.push_back( std::move(c) );
					}

					tentative_ast.push_back(std::move(a2));
				}
			}

			ast = std::move(tentative_ast);
		}
		
		for (auto &a : ast)
			all_ast.push_back(std::move(a));
	}

	// then, for each tentative ast, we tacke the recipes that are front-recursive
	{
		std::vector<SyntaxTree> all_expanded_ast;
		
		while (not all_ast.empty()) {

			SyntaxTree ast_to_expand = std::move(all_ast.back());
			all_ast.pop_back();

			all_expanded_ast.push_back(ast_to_expand);
			
			for (auto &recipe : grammar.symbols[target].recipes) {
								
				// skip non front-recursive recipes.
				if (recipe.front() != target) continue;
				
				std::vector<SyntaxTree> ast;
				ast.emplace_back(token_it, target);
				ast.back().last = token_it;
				
				for (auto component : recipe) {
					
					std::vector<SyntaxTree> tentative_ast;

					for (auto &a : ast) {
						
						if (a.last == token_it) {
						
							tentative_ast.push_back(ast_to_expand);
							continue;
						}

						for (auto &c : parse(a.last, last_token, component, debug)) {
						
							auto a2 = a;
							a2.last = c.last;
							if (component.front() == '!' and component.size()>1) {
								a2.symbol = c.symbol.substr(1);
							} else if (c.symbol != "ERASED") {
								a2.children.push_back( std::move(c) );
							}
							tentative_ast.push_back(std::move(a2));
						}
					}
					
					ast = std::move(tentative_ast);
				}
				
				for (auto &a : ast)
					all_ast.push_back(std::move(a));
			}			
		}
		
		all_ast = all_expanded_ast;
	}

	if (grammar.reducible_symbols.count(target)) {
		for (auto &ast : all_ast) {
			if (ast.symbol != target) {
				Log(DEBUG) << ast.symbol << " " << target;
				continue;
			}
			if (ast.children.empty()) {
				ast.symbol = "ERASED";
			}
			if (ast.children.size() == 1) {
				auto e = ast.last;
				ast = std::move(ast.children.front());
				ast.last = e;
			}
		}
	}
	
	return all_ast;
}

SyntaxTree::SyntaxTree(TI _first, std::string _symbol) : first(_first), last(++ _first), symbol(_symbol) {}


SyntaxTree::SyntaxTree(SourceFile &file, std::string root) {

	auto &tokens = tokenize(file);
	
	ParseDebug debug; 
	debug.last_error_token = tokens.begin();
	auto all_ast = parse(tokens.begin(), tokens.end(), root, debug);
	
	if (all_ast.empty()) {

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
	
	{
		auto last_token = all_ast.front().last;
		for (auto &ast : all_ast) if (ast.last>last_token) last_token = ast.last;

		if (last_token != tokens.end()) 
			Log(ERROR) << "Not all tokens used. Last token in: \n "  << all_ast.back().to_string() << "\n" << last_token->to_line_string();

		int count = 0;
		for (auto &ast : all_ast) if (ast.last == last_token) count++;
		

		if (count!=1) {
			for (auto &ast : all_ast) {
				std::cout << ast.to_string();
			}
			Log(ERROR) << count << " ambiguous AST";
		}

		for (auto &ast : all_ast) if (ast.last == last_token) *this = ast;
	}
}
	
