#include "ast.h"
#include "parser.h"

#include <set>


std::string SyntaxTree::to_string(std::string prefix) const {
	
	std::ostringstream oss;
	if (symbol == "IDENTIFIER") { oss << prefix << "- " << symbol << " " << first->to_string() << std::endl;
	} else { oss << prefix << "- " << symbol << std::endl;
	}
	for (auto &c : children) {
		oss << prefix + "  |" << std::endl;
		oss << c.to_string(prefix + (&c == &children.back()?"   ":"  |"));
	}
	return oss.str();
}

SyntaxTree::SyntaxTree(SourceFile &file) {

	auto &tokens = tokenize(file);
	
	ParseDebug debug; 
	debug.last_error_token = tokens.begin();
	auto all_ast = parse(tokens.begin(), tokens.end(), "start", debug);
	
	if (all_ast.empty()) {

		std::set<std::string> unique_targets;
		for (auto &target : debug.expected_targets) 
			unique_targets.insert(target);
		
		std::ostringstream oss;
		for (auto &target : unique_targets) {
			if (&target != &*unique_targets.begin()) oss << " ";
			oss << "'" << target << "'";
		}
		
		if (debug.last_error_token != tokens.end()) {
			auto &token = debug.last_error_token;
			Log(ERROR) << "Parser failed in line " << token->begin_ptr.get_line() << ". Expecting any of " << oss.str() << " but found " << token->to_string() << ".\n" << token->to_line_string();
		} else {
			Log(ERROR) << "Parser failed. Expecting any of " << oss.str() << " but the file ended";
		}
	}
	
	{
		auto last_token = all_ast.front().last;
		for (auto &ast : all_ast) if (ast.last>last_token) last_token = ast.last;

		if (last_token != tokens.end()) 
			Log(ERROR) << "Not all tokens used. Last token in: \n "  << all_ast.back().to_string() << "\n" << last_token->to_line_string();

		int count = 0;
		for (auto &ast : all_ast) if (ast.last == last_token) count++;
		

		if (count!=1) {
			for (auto &ast : all_ast) {
				std::cout << ast.to_string();
			}
			Log(ERROR) << count << " ambiguous AST";
		}

		for (auto &ast : all_ast) if (ast.last == last_token) *this = ast;
	}
}
	
