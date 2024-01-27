#pragma once
#include <tokenizer.h>
#include <grammar.h>

struct SyntaxTree {

	using TI = std::vector<Token>::const_iterator;
	
	TI first, last;

	Grammar::Symbol::Component component;
	
	std::vector<SyntaxTree> children;

	bool is_empty = false;
	
	SyntaxTree(TI _first, Grammar::Symbol::Component _component) : first(_first), last(++ _first), component(_component) {}

	SyntaxTree(SourceFile &file);
	
	std::string to_string(std::string prefix = "") const;
};

