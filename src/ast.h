#pragma once
#include <tokenizer.h>
#include <grammar.h>

struct SyntaxTree {

	struct SP : std::shared_ptr<SyntaxTree> {
		SP() {}
		SP(const std::shared_ptr<SyntaxTree> &s) : std::shared_ptr<SyntaxTree>(s) {}

		SyntaxTree::SP &operator[](size_t i) { 
			auto &ast = *this;
			if ( ast->children.size() > i ) return ast->children[i]; 
			Log(ERROR) << "Children " << i << " not found in " << ast->component.str() << ". \n" << ast->first->to_line_string(); throw;
		}

		SyntaxTree::SP &operator[](std::string s) { 
			auto &ast = *this;
			for (auto &c : ast->children) if (c and c->component.str()==s ) return c; 
			Log(ERROR) << s << " not found in " << ast->component.str() << ". \n" << ast->first->to_line_string(); throw;
		}
	};

	using TI = std::vector<Token>::const_iterator;
	
	TI first, last;

	Grammar::Symbol::Component component;

	SP old;
	SP parent;
	std::vector<SP> children;

	bool is_empty = false;
	
	SyntaxTree(TI _first, Grammar::Symbol::Component _component, SP _parent) : first(_first), last(++ _first), component(_component), parent(_parent) {}

	SyntaxTree(SourceFile &file);
	
	std::string to_string(std::string prefix = "") const;




	// filled during first pass:
	struct SymbolMap { SyntaxTree::SP symbol, type; std::string generated_name; };
	std::map<std::string, SymbolMap> symbols;
	std::map<std::string, SyntaxTree::SP> attributes;

	// filled during code generation
	std::string generated_variable_id;
};

