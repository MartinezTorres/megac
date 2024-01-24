#include "parser.h"
#include "grammar.h"
#include "log.h"


static Grammar grammar;

std::vector<SyntaxTree> parse(SyntaxTree::TI token_it, SyntaxTree::TI last_token, std::string target, ParseDebug &debug, std::vector<std::string> parent_targets_without_consuming_tokens) {

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

		auto label_error = [&](){
			if (debug.last_error_token < token_it) {
				debug.last_error_token = token_it;
				debug.expected_targets.clear();
			}
			if (debug.last_error_token == token_it) {
				debug.expected_targets.push_back(target);
			}
			return std::vector<SyntaxTree>();
		};
		
		if (token_it == last_token) 
			return label_error();

		auto expect = [&](bool condition) -> std::vector<SyntaxTree> {
			if (not condition) 
				return label_error();
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
						for (auto &c2 : c.children) 
							a2.children.push_back( std::move(c2) );
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
								for (auto &c2 : c.children) 
									a2.children.push_back( std::move(c2) );
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
