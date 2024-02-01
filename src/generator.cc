#include "generator.h"
#include <filesystem>

static struct {

	static std::vector<SyntaxTree::SP> get_translation_units(SyntaxTree::SP ast) {
		
		std::vector<SyntaxTree::SP> translation_units;
		{
			auto st = ast->parent;
			while (st) {
				if (st->component.str() == "translation_unit") {
					translation_units.push_back(st);
				}
				st = st->parent;
			}
		}
		return translation_units;
	}

	std::map<std::string, std::function<void(SyntaxTree::SP)>> processors = {

		{"translation_unit", [&](SyntaxTree::SP ast) { 

			for (auto &c : ast->children) 
				process(c);
				
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

			std::cout << included_syntax_tree->to_string();

			process(included_syntax_tree);


			return;
		}},

		{"function_definition", [&](SyntaxTree::SP ast) { 

			auto translation_units = get_translation_units(ast);
			
			if (ast->children[1]->component.str() != "function_name") 
				Log(ERROR) << "Malformed function definiton in: " << " " << ast->first->to_line_string();
			
			std::string function_name =  ast->children[1]->children[0]->first->literal;

			if (translation_units.front()->symbols.count(function_name)) 
				Log(ERROR) << "Symbol " << function_name << " already defined in scope. \n" 
					<< "First definition in: " << translation_units.front()->symbols[function_name]->first->to_line_string() 
					<< "Duplicated definition in: " << ast->first->to_line_string();

			translation_units.front()->symbols[function_name] = ast;


			return;
		}},

		{"namespace", [&](SyntaxTree::SP ast) { 

			if (ast->children[1]->component.str() != "translation_unit") 
				Log(ERROR) << "Malformed namespace definiton in: " << " " << ast->first->to_line_string();

			process(ast->children[1]);

			return;
		}},

		{"declaration", [&](SyntaxTree::SP ast) { 

			if (ast->children[1]->component.str() != "init_declarator_list") 
				Log(ERROR) << "Malformed declaration in: " << ast->first->to_line_string();

			for (auto &c : ast->children[1]->children) 
				process(c);

			return;
		}},

		{"_default_declarator", [&](SyntaxTree::SP ast) { 

			auto translation_units = get_translation_units(ast);
						
			std::string symbol_name =  ast->first->literal;

			if (translation_units.front()->symbols.count(symbol_name)) 
				Log(ERROR) << "Symbol " << symbol_name << " already defined in scope. \n" 
					<< "First definition in: " << translation_units.front()->symbols[symbol_name]->first->to_line_string() 
					<< "Duplicated definition in: " << ast->first->to_line_string();

			translation_units.front()->symbols[symbol_name] = ast;

			return;
		}},


	};

	void process(SyntaxTree::SP ast) {

		std::string symbol = ast->component.str();

		if (processors.count(symbol)==0) 
			Log(ERROR) << "There is no first pass processor for symbol \"" << symbol << "\"";
		
		processors[symbol](ast);
	}


} first_pass;





void generate_code( SyntaxTree::SP ast ) {

	if (ast->component.str() != "translation_unit") 
		Log(ERROR) << "Base syntax tree isn't translation unit but: " << ast->component.str();
	
	first_pass.process(ast);
	
}
