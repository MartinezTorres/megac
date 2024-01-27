#include "generator.h"
#include <filesystem>

static struct {

	std::map<std::string, std::function<void(SyntaxTree &)>> processors = {

		{"translation_unit", [&](SyntaxTree &ast) { 

			for (auto &c : ast.children) {
				process(c.component.str(), c);
			}
			return;

		}},

		{"include", [&](SyntaxTree &ast) { 

			Assert(ast.children.size()==1) << " include lacks a parameter.";
			Assert(ast.children.front().component.str()=="STRING_LITERAL") << " parameter isn't a STRING_LITERAL";

			std::filesystem::path ast_file_path = ast.first->begin_ptr.get_file().path;
			std::string included_file_name = std::string(ast_file_path.parent_path()) + "/" + ast.children.front().first->literal;

			Log(INFO) << "Including file: " << included_file_name;

			SourceFile &included_source_file = SourceFile::Manager::get(included_file_name);

			SyntaxTree included_ast( included_source_file );
			ast = included_ast;
			return;

		}},


	};

	void process(std::string symbol, SyntaxTree &ast) {

		if (processors.count(symbol)==0) 
			Log(ERROR) << "There is no first pass processor for symbol \"" << symbol << "\"";
		
		processors[symbol](ast);
	}


} first_pass;





void generate_code( SyntaxTree ast ) {

	if (ast.component.str() != "translation_unit") 
		Log(ERROR) << "Base syntax tree isn't translation unit but: " << ast.component.str();
	
	first_pass.process("translation_unit", ast);
	
}
