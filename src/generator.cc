#include "generator.h"
#include <filesystem>
#include <functional>

/////////////////////////////////////////////////////////////////
// COMMON HELPERS
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

static void register_symbol(SyntaxTree::SP &ast, SyntaxTree::SP &type,const std::string &name) {

	static uint32_t symbol_idx = 1000000;
	
	auto translation_unit = ast;
	do {

		Log(INFO) << translation_unit->component.str()  << " " << name;
		translation_unit = translation_unit->parent;
	
	} while (translation_unit->component.str() != "translation_unit");

	if (translation_unit->symbols.count(name)) 
		Log(ERROR) << "Symbol " << name << " already defined in scope. \n" 
			<< "First definition in: " << translation_unit->symbols[name].symbol->first->to_line_string() 
			<< "Duplicated definition in: " << ast->first->to_line_string();

	translation_unit->symbols[name] = { ast, type, "____mc" + std::to_string(symbol_idx++) + name};
}


/////////////////////////////////////////////////////////////////
// PREPROCESSOR PASS: 
//  * TARGETS: INCLUDE MODULES, SIMPLIFY SYNTAX
static struct {

	std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

		{"include", [&](SyntaxTree::SP &ast) { 

			Assert(ast[0]->component.str()=="STRING_LITERAL") << " parameter isn't a STRING_LITERAL";

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

		{"translation_unit_single", [&](SyntaxTree::SP &ast) { ast->component = Grammar::Symbol::Component::Symbol("translation_unit"); }},

		{"function_definition", [&](SyntaxTree::SP &ast) { 

			// We leverage function definintions as in C format for convenience, but this does not match very well our definition of function type, 
			// so we modify the order of the function childrens to match the function type definintion
			std::swap( ast->children[1], ast->children[2] );
		}},

	/*	{"foreach", [&](SyntaxTree::SP &ast) { 

			SyntaxTree::SP new_ast;
			new_ast->parent = ast->parent;
			new_ast->old = ast;
			new_ast->component = Grammar::Symbol::Component::Symbol("included_scope");
			ast = new_ast;

			auto iterator = ast.old[0];
			auto container = ast.old[1];

		}},*/
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

	std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {


		{"function_definition", [&](SyntaxTree::SP &ast) { 
			
			std::string function_name =  ast[2][0]->first->literal;

			SyntaxTree::SP type = ast->parent;

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
// There is no optimizations here, only raw code generation. Not even inling.


struct CodeGenerationPass {

	std::map<std::string, std::ostringstream> code_units;

	uint32_t unit_name_idx = 1000000;
	std::string new_id() {

		return "id_" + std::to_string(unit_name_idx++);
	}

	std::string type_to_c(const SyntaxTree::SP &) {

		std::string ret = "";

		return ret;

	}	
	
	std::map<std::string, std::function<bool(SyntaxTree::SP &, std::ostringstream &)>> processors = {



		{"function_call", [&](SyntaxTree::SP &ast, std::ostringstream &oss) -> bool  { 

			NamespacedIdentifier function_name(ast->children[0]);
			auto funtion_ast = function_name.resolve(ast);

			std::cout << funtion_ast.type->to_string();

			auto &args = funtion_ast.type[0][1]->children;
			for (auto &arg : args)
				process( arg, oss );

			auto return_type = funtion_ast.type[0][0][0][0];
			if (return_type->component.str() != "void") {
				ast->generated_variable_id = new_id();
				oss << type_to_c(return_type) << " " << ast->generated_variable_id << " = ";
			}

			oss << funtion_ast.generated_name << "( " ;
			for ( auto &a : args ) {

				if ( &a != &args.front() ) oss << ", ";
				oss << a->generated_variable_id;
			}
			oss << ");" << std::endl;

			return false;
		}},
	};

	void process( SyntaxTree::SP &ast ) { process (ast, code_units["base"] ); }
	void process(SyntaxTree::SP &ast, std::ostringstream &oss) {

		std::string symbol = ast->component.str();

		bool process_children = true;

		if (processors.count(symbol))
			process_children = processors[symbol](ast, oss);

		if (process_children)
			for (auto &c : ast->children)
				process(c, oss);
	}

};



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
	CodeGenerationPass code_generation_pass;
	code_generation_pass.process(main_syntax_tree);
	//std::cout << main_syntax_tree->to_string();

	for (auto &cu : code_generation_pass.code_units) {

		Log(INFO) << cu.first;
		std::cout << cu.second.str() << std::endl;

	}

}
