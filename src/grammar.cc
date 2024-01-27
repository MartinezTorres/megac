#include "grammar.h"

#include <set>

extern char ext_grammar[];

Grammar::Grammar() {	

	std::istringstream iss(ext_grammar);
	std::string s;

	// Parse the first section. Fill reserved_keyword lists and magic token lists. 
	while (iss >> s) {

		if (s == "//") { 
			// skip comments
			std::getline(iss, s);

		} else if (s == "%%") {
			// end of first section.
			break;
		
		} else if (s == "%reserved_keywords") {
			// add reserved keywords
			while (iss >> s) {
				if (s == ";") break;
				reserved_keywords.insert(s);
			}
		} else if (s == "%token") {
			// add magic tokens
			while (iss >> s) {
				if (s == ";") break;
				magic_tokens.insert(s);
			}
		} else {
			Log(ERROR) << "Unknown directive in first grammar section: " << s;
		}
	}

	// Parse second section: symbol definitions and their recipes.
	while (iss >> s) {
		
		if (s == "//") { 
			// skip comments
			std::getline(iss, s);
			continue;

		} 

		bool is_weak = false;
		while (s.front()=='%') {
			if (s == "%weak") {
				is_weak = true;
			} else {
				Log(ERROR) << "Unknown directive in second grammar section: " << s;
			}
			if (not (iss >> s) ) return;
		}

		Symbol &symbol = symbols[s];
		symbol.name = s;
		symbol.is_weak = is_weak;
		is_weak = false;

		// Consume ':'
		{
			if (not (iss >> s) ) return;
			Assert(s==":") << ": expected, got " << s;
		}
		
		std::vector<std::vector<Symbol::Component>> recipes;
		std::string subsymbol_name;
		bool force_root = false;
		bool must_keep = false;
		bool is_optional = false;
		while (iss >> s) {
			
			if (s=="|" or s==";") { // recipe is finished.
				
				if (subsymbol_name.empty()) {
					
					for (auto &recipe : recipes) {
						Assert(not recipe.empty()) << " found an empty recipe in symbol " << symbol.name;
						symbol.recipes.push_back(recipe);
					}
					recipes.clear();
				} else {

					Grammar::Symbol::Component phony_component;
					phony_component = Grammar::Symbol::Component::Symbol(symbol.name + "_sub_" + subsymbol_name);
					
					symbol.recipes.emplace_back(1,phony_component);
					
					Symbol &subsymbol = symbols[phony_component.str()];
					subsymbol.name = subsymbol_name;
					subsymbol.is_weak = false;

					for (auto &recipe : recipes) {
						Assert(not recipe.empty()) << " found an empty recipe in symbol " << subsymbol_name;
						//Log(ERROR_NOTHROW) << "Subsymbol: " << subsymbol_name << " recipe: " << ([&](){std::ostringstream oss; for (auto &v:recipe) oss<<v<<" "; return oss.str();}());
						subsymbol.recipes.push_back(recipe);
					}
					recipes.clear();
											
					subsymbol_name.clear();
				}
				if (s==";") break;
				
			} else if (s == "%label") {

				if (not (iss >> s) ) Log(ERROR) << " expecting <name> after %label";
			
				subsymbol_name = s;
				 
			} else if (s == "%root") {

				force_root = true;

			} else if (s == "%keep") {

				must_keep = true;
			
			} else if (s == "%opt") {

				is_optional = true;
			
			} else {
				
				if (recipes.empty()) recipes.emplace_back();

				Symbol::Component component;

				if (s.front() == '\'') {
					component = Grammar::Symbol::Component::Token(s.substr(1,s.size()-2));
				} else {
					component = Grammar::Symbol::Component::Symbol(s);
				}
				component.force_root = force_root;
				force_root = false;

				component.must_keep = must_keep;
				must_keep = false;

				if (is_optional) {
					
					auto new_recipes = recipes;
					for (auto &recipe : new_recipes) {

						recipe.push_back(component);
						recipes.push_back(recipe);
					}
				} else {
					for (auto &recipe : recipes) {
						recipe.push_back(component);
					}
				}

				is_optional = false;
			}
		}
	}
}

asm("ext_grammar:    .incbin \"grammar.y\" \n .balign 1 \n .byte 0x00\n");
