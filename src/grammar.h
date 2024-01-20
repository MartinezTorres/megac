#pragma once
#include <tokenizer.h>

#include <set>

struct Grammar {
	
	

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
	
	Grammar();
};
