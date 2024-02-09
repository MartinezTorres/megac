#include "generator.h"
#include <filesystem>
#include <functional>


/////////////////////////////////////////////////////////////////
// PREPROCESSOR PASS: 
//  * TARGET: INCLUDE MODULES

static struct {

	std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

		{"include", [&](SyntaxTree::SP &ast) { 

			Assert(ast->children.size()==1) << " include lacks a parameter.";
			Assert(ast->children.front()->component.str()=="STRING_LITERAL") << " parameter isn't a STRING_LITERAL";

			std::filesystem::path ast_file_path = ast->first->begin_ptr.get_file().path;
			std::string included_file_name = std::string(ast_file_path.parent_path()) + "/" + ast->children.front()->first->literal;

			Log(INFO) << "Including file: " << included_file_name;

			SourceFile &included_source_file = SourceFile::Manager::get(included_file_name);

			SyntaxTree::SP included_syntax_tree = std::make_shared<SyntaxTree>( included_source_file );

			included_syntax_tree->parent = ast->parent;
			included_syntax_tree->old = ast;
			included_syntax_tree->component = Grammar::Symbol::Component::Symbol("included_scope");
						
			ast = included_syntax_tree;

		}},
	};

	void process(SyntaxTree::SP &ast) {

		std::string symbol = ast->component.str();

		if (processors.count(symbol))
			processors[symbol](ast);

		for (auto &c : ast->children)
			c->parent = ast;

		for (auto &c : ast->children)
			process(c);
	}

} preprocessor_pass;


/////////////////////////////////////////////////////////////////
// INDENTIFY SYMBOLS AND ATTRIBUTES PASS: 

static struct {

	void register_symbol(SyntaxTree::SP &ast, SyntaxTree::SP &type,const std::string &name) {
		
		auto translation_unit = ast;
		do {

			Log(INFO) << translation_unit->component.str()  << " " << name;
			translation_unit = translation_unit->parent;
		
		} while (translation_unit->component.str() != "translation_unit");

		if (translation_unit->symbols.count(name)) 
			Log(ERROR) << "Symbol " << name << " already defined in scope. \n" 
				<< "First definition in: " << translation_unit->symbols[name].symbol->first->to_line_string() 
				<< "Duplicated definition in: " << ast->first->to_line_string();

		translation_unit->symbols[name] = { ast, type };
	}

	std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

		{"statement_scope", [&](SyntaxTree::SP &ast) { ast->component = Grammar::Symbol::Component::Symbol("translation_unit"); }},

		{"function_definition", [&](SyntaxTree::SP &ast) { 

			if (ast->children[1]->component.str() != "function_name") 
				Log(ERROR) << "Malformed function definiton in: " << " " << ast->first->to_line_string();
			
			std::string function_name =  ast->children[1]->children[0]->first->literal;

			SyntaxTree::SP type = ast;

			register_symbol( ast, type, function_name );
		}},

		{"type_declaration", [&](SyntaxTree::SP &ast)  { 
						
			for (std::size_t i = 1; i < ast->children.size(); i++ ) {

				auto &c = ast->children[i];
				auto &type = ast->children[0];

				std::string symbol_name =  c->first->literal;
				register_symbol( ast, type, symbol_name );
			}
		}},

		{"namespace", [&](SyntaxTree::SP &ast)  { 
						
			std::string namespace_name = ast->children[0]->first->literal;

			register_symbol( ast->children[1], ast, namespace_name );
		}},

		{"attributes", [&](SyntaxTree::SP &ast)  { 

			std::map<std::string, SyntaxTree::SP> &attributes = ast->parent->attributes;
		
			for ( auto &attribute : ast->children ) {

				if (attribute->component.str() == "=" ) {

					std::string name = attribute->children[0]->first->literal;
					if (attributes.count(name)) 
						Log(ERROR) << "Attribute " << name << " already defined. \n" << attribute->children[0]->first->to_line_string();

					attributes.emplace( name, attribute->children[1] );
				} else {

					std::string name = attribute->first->literal;
					attributes.emplace( name, attribute );
				}
			}
		}},
	};

	void process(SyntaxTree::SP &ast) {

		std::string symbol = ast->component.str();

		if (processors.count(symbol))
			processors[symbol](ast);

		for (auto &c : ast->children)
			c->parent = ast;

		for (auto &c : ast->children)
			process(c);
	}

} identify_symbols_pass;


/////////////////////////////////////////////////////////////////
// OPTIMIZER PASS: 


static struct {

	std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

	};

	void process(SyntaxTree::SP &ast) {

		std::string symbol = ast->component.str();

		if (processors.count(symbol))
			processors[symbol](ast);

		for (auto &c : ast->children)
			process(c);
	}

} optimizer_pass;


/////////////////////////////////////////////////////////////////
// CODE GENERATION PASS: 


static struct {

	struct NamespacedIdentifier : public std::vector<std::string> {

		NamespacedIdentifier(SyntaxTree::SP &ast) {

			if (ast->component.str() != "namespaced_identifier") 
				Log(ERROR) << "Missing namespaced_identifier in: " << " " << ast->first->to_line_string();
			
			for (auto &c : ast->children) 
				push_back(c->first->literal);
		}

		SyntaxTree::SymbolMap &resolve( const SyntaxTree::SP &ast ) { return resolve(ast, ast); }

		SyntaxTree::SymbolMap &resolve(const SyntaxTree::SP &ast, const SyntaxTree::SP &from) {
			
			if (size()==1) {

				if (ast->symbols.count(front())) return ast->symbols[front()];
				if (!ast->parent) Log(ERROR) << "Symbol " << front() << " not found. Needed from: \n" << from->first->to_line_string();
				return resolve(ast->parent, from);
			}

			SyntaxTree::SP a = ast;
			for (auto &s : *this) {
				if (a->symbols.count(s) == 0) 
					break;

				if ( &s == &back() ) 
					return a->symbols[s];
				
				a = a->symbols[s].symbol;
			}

			if (!ast->parent) Log(ERROR) << "Symbol " << front() << " not found. Needed from:\n" << from->first->to_line_string();

			return resolve(ast->parent, from);
		}
	};


	std::map<std::string, std::function<void(SyntaxTree::SP &, std::map<std::string, std::ostringstream> &, const std::string &)>> processors = {

		{"function_call", [&](SyntaxTree::SP &ast, std::map<std::string, std::ostringstream> &, const std::string &)  { 

			NamespacedIdentifier function_name(ast->children[0]);
			auto funtion_ast = function_name.resolve(ast);





			
		}},
	};

	void process(SyntaxTree::SP &ast, std::map<std::string, std::ostringstream> &out, const std::string &base) {

		std::string symbol = ast->component.str();

		if (processors.count(symbol))
			processors[symbol](ast, out, base);

		for (auto &c : ast->children)
			process(c, out, base);
	}

} code_generation_pass;



void generate_code( std::string source_file_name ) {

	SourceFile &source_file = SourceFile::Manager::get(source_file_name);

	SyntaxTree::SP main_syntax_tree = std::make_shared<SyntaxTree>( source_file );
	

	if (main_syntax_tree->component.str() != "translation_unit") 
		Log(ERROR) << "Base syntax tree isn't translation unit but: " << main_syntax_tree->component.str();
	
	Log(INFO) << "PREPROCESSOR PASS";
	preprocessor_pass.process(main_syntax_tree);
	std::cout << main_syntax_tree->to_string();

	Log(INFO) << "SYMBOL IDENTIFICATION PASS";
	identify_symbols_pass.process(main_syntax_tree);
	std::cout << main_syntax_tree->to_string();

	Log(INFO) << "SECOND PASS";
	optimizer_pass.process(main_syntax_tree);
	std::cout << main_syntax_tree->to_string();

	Log(INFO) << "THIRD PASS";
	std::map<std::string, std::ostringstream> out;
	code_generation_pass.process(main_syntax_tree, out, "");
	std::cout << main_syntax_tree->to_string();

}
