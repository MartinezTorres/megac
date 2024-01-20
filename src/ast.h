#pragma once
#include <tokenizer.h>

struct SyntaxTree {

	using TI = std::vector<Token>::const_iterator;
	
	TI first, last;
	std::string symbol;
	
	std::vector<SyntaxTree> children;
	
	SyntaxTree(TI _first, std::string _symbol) : first(_first), last(++ _first), symbol(_symbol) {}

	SyntaxTree(SourceFile &file);
	
	std::string to_string(std::string prefix = "") const;
};

