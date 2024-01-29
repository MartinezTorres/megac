#include "generator.h"
#include <filesystem>

static struct {

	std::map<std::string, std::function<void(SyntaxTree::SP)>> processors = {

		{"translation_unit", [&](SyntaxTree::SP ast) { 

			for (auto &c : ast->children) {
				if (c) {
					process(c->component.str(), c);
				}
			}
			return;

		}},

		{"include", [&](SyntaxTree::SP ast) { 

			Assert(ast->children.size()==1) << " include lacks a parameter.";
			Assert(ast->children.front()->component.str()=="STRING_LITERAL") << " parameter isn't a STRING_LITERAL";

			std::filesystem::path ast_file_path = ast->first->begin_ptr.get_file().path;
			std::string included_file_name = std::string(ast_file_path.parent_path()) + "/" + ast->children.front()->first->literal;

			Log(INFO) << "Including file: " << included_file_name;

			SourceFile &included_source_file = SourceFile::Manager::get(included_file_name);

			SyntaxTree::SP included_syntax_tree = std::make_shared<SyntaxTree>( included_source_file );

			included_syntax_tree->parent = ast;
			ast->included_syntax_tree = included_syntax_tree;
			
			return;
		}},


	};

	void process(std::string symbol, SyntaxTree::SP ast) {

		if (processors.count(symbol)==0) 
			Log(ERROR) << "There is no first pass processor for symbol \"" << symbol << "\"";
		
		processors[symbol](ast);
	}


} first_pass;





void generate_code( SyntaxTree::SP ast ) {

	if (ast->component.str() != "translation_unit") 
		Log(ERROR) << "Base syntax tree isn't translation unit but: " << ast->component.str();
	
	first_pass.process("translation_unit", ast);
	
}
