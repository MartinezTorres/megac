#pragma once
#include <tokenizer.h>

#include <set>

struct Grammar {
	
	

	// A grammar symbol can be build from one of many possible recipes.
	struct Symbol {
		
		// Symbol name
		//std::string name;

		// Weak symbols can be removed while parsing the tree if they have less than two children
		bool is_weak;
		
		// Each symbol recipe is a list of different components
		struct Component {
		private:
			enum { SYMBOL, TOKEN } type;
			std::string _str;
		public:
			bool force_root = false;
			bool must_keep = false;
			
			static Component Symbol(std::string s) { Component c; c._str = s; c.type = SYMBOL; return c; }
			static Component Token(std::string s) { Component c; c._str = s; c.type = TOKEN; return c; }

			const std::string &str() const { return _str; } 
			bool is_symbol() const { return type == SYMBOL;}
			bool is_token() const { return type == TOKEN;}
		};
		std::vector<std::vector<Component>> recipes;
	};
	
	std::set<std::string> reserved_keywords, magic_tokens;

	std::map<std::string, Symbol> symbols;
	
	Grammar();
};
