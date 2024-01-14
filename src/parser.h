#pragma once
#include <tokenizer.h>

#include <optional>

struct SyntaxTree {
	

	using TI = std::vector<Token>::const_iterator;
	
	TI begin, end;
	std::string symbol;
	
	std::vector<SyntaxTree> children;
	
	SyntaxTree(TI begin, std::string symbol);

	SyntaxTree(SourceFile &file, std::string root = "translation_unit");
	
	std::string to_string(std::string prefix = "") const;
};

