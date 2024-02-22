#include "parser.h"
#include "grammar.h"
#include "log.h"

static Grammar grammar;

std::vector<SyntaxTree::SP> parse(SyntaxTree::TI token_it, SyntaxTree::TI last_token, Grammar::Symbol::Component target, ParseDebug &debug, std::vector<std::string> parent_targets_without_consuming_tokens, SyntaxTree::SP parent) {

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
					return std::vector<SyntaxTree::SP>();
		}
	}

	// tackle leaf nodes: which can be a keyword, an identifier, a numeric constant, or a string literal.
	{
		auto expect = [&](bool condition) -> std::vector<SyntaxTree::SP> {
			if (not condition) {

				if (debug.last_error_token < token_it) {
					debug.last_error_token = token_it;
					debug.expected_targets.clear();
				}
				if (debug.last_error_token == token_it) {
					debug.expected_targets.push_back(target.id());
				}
				return std::vector<SyntaxTree::SP>();				
			}
			
			SyntaxTree::SP ret = std::make_shared<SyntaxTree>(token_it, target, parent);

			if (target.is_token() and not target.must_keep)
				ret->is_empty = true;

			if (target.must_keep) {
				ret->component = Grammar::Symbol::Component::Symbol( token_it->literal );
			}

			return std::vector<SyntaxTree::SP>(1, ret);
		};

		// Take care if we are past the final token.
		if (token_it == last_token) 
			return expect( false );

		// Also there are some leaf nodes harcoded in the symbol table
		if ( target.is_symbol() and target.id() == "IDENTIFIER" ) 
			return expect( (token_it->type == Token::IDENTIFIER) and (grammar.reserved_keywords.count(token_it->literal) == 0) );

		if ( target.is_symbol() and target.id() == "CONSTANT" ) 
			return expect(token_it->type == Token::NUMERIC);
		
		if ( target.is_symbol() and target.id() == "STRING_LITERAL" ) 
			return expect(token_it->type == Token::STRING_LITERAL);

		if ( target.is_token() ) 
			return expect( (token_it->type != Token::STRING_LITERAL) and (token_it->literal == target.id()) );

	} 

	
	// for each target symbol, there may be several recipes possible.
	std::vector<SyntaxTree::SP> all_ast;

	// register our current target.
	parent_targets_without_consuming_tokens.push_back(target.id());

	// for each symbol, we tackle first the recipes that aren't front-recursive
	for (auto &recipe : grammar.symbols[target.id()].recipes) {

		// skip pure front-recursive recipes.
		{
			bool is_front_recursive_recipe = recipe.front().is_symbol() and (recipe.front().id() == target.id());
			if (is_front_recursive_recipe) continue;
		}

		std::vector<SyntaxTree::SP> ast;
		ast.push_back(std::make_shared<SyntaxTree>(token_it, target, parent));
		ast.back()->last = token_it;

		for (auto component : recipe) {

			std::vector<SyntaxTree::SP> tentative_ast;
			
			for (auto &a : ast) {

				
				bool is_first = ( a->last == token_it );
				
				auto &&t = (is_first?parent_targets_without_consuming_tokens:std::vector<std::string>()); 

				for (auto &c : parse(a->last, last_token, component, debug, t)) {
				
					auto a2 = std::make_shared<SyntaxTree>(*a);
					a2->last = c->last;
					for (auto &a2c : a2->children)
						a2c->parent = a2;

					if (component.force_root) {
						a2->component = Grammar::Symbol::Component::Symbol( c->component.id() );
						for (auto &c2 : c->children) {
							c2->parent = a2;
							a2->children.push_back( c2 ) ;
						}
					} else if (c->is_empty == false) {
						c -> parent = a2;
						a2->children.push_back( c );
					}

					tentative_ast.push_back( a2 );
				}
			}

			ast = tentative_ast;
		}
		
		for (auto &a : ast)
			all_ast.push_back(a);
	}

	// then, for each tentative ast, we tacke the recipes that are front-recursive
	{
		std::vector<SyntaxTree::SP> all_expanded_ast;
		
		while (not all_ast.empty()) {

			SyntaxTree::SP ast_to_expand = all_ast.back();
			all_ast.pop_back();

			all_expanded_ast.push_back(ast_to_expand);
			
			for (auto &recipe : grammar.symbols[target.id()].recipes) {
								
				// skip non front-recursive recipes.
				{
					bool is_front_recursive_recipe = recipe.front().is_symbol() and (recipe.front().id() == target.id());
					if (not is_front_recursive_recipe) continue;
				}
				
				std::vector<SyntaxTree::SP> ast;
				ast.push_back(std::make_shared<SyntaxTree>(token_it, target, parent));
				ast.back()->last = token_it;
				
				for (auto component : recipe) {
					
					std::vector<SyntaxTree::SP> tentative_ast;

					for (auto &a : ast) {
						
						if (a->last == token_it) {
						
							tentative_ast.push_back(std::make_shared<SyntaxTree>( *ast_to_expand ) );
							continue;
						}

						for (auto &c : parse(a->last, last_token, component, debug)) {
						
							auto a2 = std::make_shared<SyntaxTree>(*a);
							a2->last = c->last;
							for (auto &a2c : a2->children)
								a2c->parent = a2;

							if (component.force_root) {
								a2->component = Grammar::Symbol::Component::Symbol( c->component.id() );
								for (auto &c2 : c->children) {
									c2->parent = a2;
									a2->children.push_back( c2 ) ;
								}
							} else if (c->is_empty == false) {
								c -> parent = a2;
								a2->children.push_back( c );
							}
							tentative_ast.push_back( a2 );
						}
					}
					
					ast = tentative_ast;
				}
				
				for (auto &a : ast)
					all_ast.push_back(a);
			}			
		}
		
		all_ast = all_expanded_ast;
	}

	if (grammar.symbols[target.id()].is_weak) {
		for (auto &ast : all_ast) {
			if (ast->component.id() != target.id()) {
				continue;
			}
			if (ast->children.empty()) {
				ast->is_empty = true;
			}
			if (ast->children.size() == 1) {
				auto e = ast->last;
				auto p = ast->parent;
				ast = ast->children.front();
				ast->last = e;
				ast->parent = p;
			}
		}
	}
	
	return all_ast;
}
