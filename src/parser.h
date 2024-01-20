#pragma once
#include <ast.h>

struct ParseDebug {
	SyntaxTree::TI last_error_token;
	std::vector<std::string> expected_targets;
};

std::vector<SyntaxTree> parse(SyntaxTree::TI token_it, SyntaxTree::TI last_token, std::string target, ParseDebug &debug, std::vector<std::string> parent_targets_without_consuming_tokens = std::vector<std::string>());
