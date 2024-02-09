#pragma once
#include <tokenizer.h>
#include <grammar.h>

struct SyntaxTree {

	using SP = std::shared_ptr<SyntaxTree>;
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
	struct SymbolMap { SyntaxTree::SP symbol, type; };
	std::map<std::string, SymbolMap> symbols;
};

