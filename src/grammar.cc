#include "grammar.h"

#include <set>

extern char ext_grammar[];

Grammar::Grammar() {	
	
	std::istringstream iss(ext_grammar);
	std::string s;

	// Let's make a list of found symbols first
	while (iss >> s) {
		
		if (s.front()=='#') { while (iss >> s) if (s == ";") break; iss >> s; }
		
		symbols[s];
		
		while (iss >> s) if (s == ";") break;
	}

	iss = std::istringstream(ext_grammar);
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

asm("ext_grammar:    .incbin \"grammar.y\" \n .balign 1 \n .byte 0x00\n");
