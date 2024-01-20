#pragma once
#include <tokenizer.h>

#include <optional>

struct SyntaxTree {
	

	using TI = std::vector<Token>::const_iterator;
	
	TI first, last;
	std::string symbol;
	
	std::vector<SyntaxTree> children;
	
	SyntaxTree(TI begin, std::string symbol);

	SyntaxTree(SourceFile &file, std::string root = "start");
	
	std::string to_string(std::string prefix = "") const;
};

