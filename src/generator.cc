#include "generator.h"
#include <filesystem>
#include <functional>


/////////////////////////////////////////////////////////////////
// COMMON HELPERS
struct NamespacedIdentifier : public std::vector<std::string> {

	NamespacedIdentifier(SyntaxTree::SP &ast) {

		if (ast.id() != "namespaced_identifier") 
			ast.log(ERROR) << "Missing namespaced_identifier";
		
		for (auto &c : ast->children) 
			push_back(c->first->literal);
	}

	SyntaxTree::SP &resolve( const SyntaxTree::SP &ast ) { return resolve(ast, ast); }

	SyntaxTree::SP &resolve(const SyntaxTree::SP &ast, const SyntaxTree::SP &from) {
		
		if (size()==1) {

			if (ast->symbols.count(front())) return ast->symbols[front()];
			if (!ast->parent) from.log(ERROR) << "Symbol " << front() << " not found. Needed from: ";
			return resolve(ast->parent, from);
		}

		SyntaxTree::SP a = ast;
		for (auto &s : *this) {
			if (a->symbols.count(s) == 0) 
				break;

			if ( &s == &back() ) 
				return a->symbols[s];
			
			a = a->symbols[s];
		}

		if (!ast->parent) from.log(ERROR) << "Symbol " << front() << " not found. Needed from: ";

		return resolve(ast->parent, from);
	}
};

static void register_symbol(SyntaxTree::SP &ast, const SyntaxTree::SP &type, const std::string &name) {

	auto translation_unit = ast;
	do {

		Log(INFO) << translation_unit.id()  << " " << name;
		translation_unit = translation_unit->parent;
	
	} while (translation_unit.id() != "translation_unit");

	if (translation_unit->symbols.count(name)) 
		Log(ERROR) << "Symbol " << name << " already defined in scope. \n" 
			<< "First definition in: " << translation_unit->symbols[name]->first->show_source() 
			<< "Duplicated definition in: " << ast->first->show_source();

	ast->type = type;
	ast->generated_id += name;
	translation_unit->symbols[name] = ast;
}

namespace CompilerPass {

	/////////////////////////////////////////////////////////////////
	// PREPROCESSOR PASS: 
	//  * TARGETS: INCLUDE MODULES, SIMPLIFY SYNTAX
	struct Preprocessor {

		std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

			{"include", [&](SyntaxTree::SP &ast) { 

				if (ast[0].id() != "STRING_LITERAL")
					ast.log(ERROR) << " include isn't a STRING_LITERAL";

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

			{"foreach", [&](SyntaxTree::SP &ast) { 

				// It's not that I took the terminator from stackoverflow, but I just really like Vogons.
				static const constexpr std::string_view foreach_string = R"V0G0N(
				{
					auto __mc__begin = a1.begin;
					auto __mc__end = a2.end;
					for ( ; __mc__begin != __mc__end ; ++__mc__begin )  {
						auto a3 = __mc__begin[0];
						{
							a4;
						}
					}
				}
				)V0G0N";	

				SourceFile &foreach_file = SourceFile::Manager::get("__mc__foreach_macro", foreach_string);
				SyntaxTree::SP foreach_ast = std::make_shared<SyntaxTree>( foreach_file );

				//std::cerr << foreach_ast->to_string(); 
				SyntaxTree::SP &a1 = foreach_ast[0][0][0][1][0];
				//std::cerr << "A1: \n" << a1->to_string(); 
				SyntaxTree::SP &a2 = foreach_ast[0][1][0][1][0];
				//std::cerr << "A2: \n" << a2->to_string(); 
				SyntaxTree::SP &a3 = foreach_ast[0][2][2][0][0][0][0];
				//std::cerr << "A3: \n" << a3->to_string(); 
				SyntaxTree::SP &a4 = foreach_ast[0][2][2][0][1];
				//std::cerr << "A4: \n" << a4->to_string(); 

				//std::cerr << ast->to_string(); 

				a1 = ast[1];
				a2 = ast[1];
				a3 = ast[0];
				a4 = ast[2];

				foreach_ast->parent = ast->parent;
				foreach_ast->old = ast;
				ast = foreach_ast;

				//std::cerr << ast->to_string(); 

			}},

			
			{"auto", [&](SyntaxTree::SP &ast) { 

				// It's not that I took the terminator from stackoverflow, but I just really like Vogons.
				static const constexpr std::string_view auto_string = R"V0G0N(
					typeof(a1) a2 = a3;
				)V0G0N";	

				SourceFile &auto_file = SourceFile::Manager::get("__mc__auto_macro", auto_string);
				SyntaxTree::SP auto_ast = std::make_shared<SyntaxTree>( auto_file );
				auto_ast = auto_ast[0];

				//std::cerr << auto_ast->to_string(); 
				SyntaxTree::SP &a1 = auto_ast[0][0][0];
				//std::cerr << "A1: \n" << a1->to_string(); 
				SyntaxTree::SP &a2 = auto_ast[1][0][0];
				//std::cerr << "A2: \n" << a2->to_string(); 
				SyntaxTree::SP &a3 = auto_ast[1][0][1];
				//std::cerr << "A3: \n" << a3->to_string(); 

				//std::cerr << ast->to_string(); 

				a1 = ast[0][1];
				a2 = ast[0][0];
				a3 = ast[0][1];

				auto_ast->parent = ast->parent;
				auto_ast->old = ast;
				ast = auto_ast;

				//std::cerr << ast->to_string(); 
				//Log(ERROR) << "";

			}},
		};

		void process(SyntaxTree::SP &ast) {

			std::string symbol = ast.id();

			if (processors.count(symbol))
				processors[symbol](ast);

			for (auto &c : ast->children)
				c->parent = ast;

			for (auto &c : ast->children)
				process(c);
		}

	};


	/////////////////////////////////////////////////////////////////
	// COMPILER PASS: INDENTIFY SYMBOLS AND ATTRIBUTES
	struct Symbols{

		std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {


			{"function_definition", [&](SyntaxTree::SP &ast) { 
				
				std::string function_name =  ast[2][0].literal();

				SyntaxTree::SP type = ast->parent;

				register_symbol( ast, type, function_name );
			}},

			{"type_declaration", [&](SyntaxTree::SP &ast)  { 
				
				auto &type_declaration_list = ast[1];
				for (std::size_t i = 0; i < type_declaration_list->children.size(); i++ ) {

					auto c = type_declaration_list[i];
					if ( c.id() == "=" )
						c = c[0];

					std::string symbol_name =  c->first->literal;

					auto &type = ast[0];

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

					if (attribute.id() == "=" ) {

						std::string name = attribute->children[0]->first->literal;
						if (attributes.count(name)) 
							attribute->children[0].log(ERROR) << "Attribute " << name << " already defined in: ";

						attributes.emplace( name, attribute->children[1] );
					} else {

						std::string name = attribute->first->literal;
						attributes.emplace( name, attribute );
					}
				}
			}},
		};

		void process(SyntaxTree::SP &ast) {

			std::string symbol = ast.id();

			if (processors.count(symbol))
				processors[symbol](ast);

			for (auto &c : ast->children)
				c->parent = ast;

			for (auto &c : ast->children)
				process(c);
		}

	};


	/////////////////////////////////////////////////////////////////
	// COMPILER PASS: INDENTIFY TYPES
	struct Types{

		std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

			{"void",   [&](SyntaxTree::SP &ast) { ast->c_type = "void"; }},
			{"int8",   [&](SyntaxTree::SP &ast) { ast->c_type = "int8_t"; }},
			{"uint8",  [&](SyntaxTree::SP &ast) { ast->c_type = "uint8_t"; }},
			{"int16",  [&](SyntaxTree::SP &ast) { ast->c_type = "int16_t"; }},
			{"uint16", [&](SyntaxTree::SP &ast) { ast->c_type = "uint16_t"; }},
			{"int32",  [&](SyntaxTree::SP &ast) { ast->c_type = "int32_t"; }},
			{"uint32", [&](SyntaxTree::SP &ast) { ast->c_type = "uint32_t"; }},
			{"type_name", [&](SyntaxTree::SP &ast) { 
				if (ast->children.size() != 1)
					ast.log(ERROR) << "Type name does not have a single child";
				ast->c_type = ast[0]->c_type; 
			}},
		};

		void process(SyntaxTree::SP &ast) {

			std::string symbol = ast.id();

			for (auto &c : ast->children)
				c->parent = ast;

			if (processors.count(symbol))
				processors[symbol](ast);

			for (auto &c : ast->children)
				process(c);
		}

	};


	/////////////////////////////////////////////////////////////////
	// OPTIMIZER PASS: 
	struct Optimizer {

		std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

		};

		void process(SyntaxTree::SP &ast) {

			std::string symbol = ast.id();

			if (processors.count(symbol))
				processors[symbol](ast);

			for (auto &c : ast->children)
				process(c);
		}

	};


	/////////////////////////////////////////////////////////////////
	// CODE GENERATION PASS: 
	// There is no optimizations here, only raw code generation. Not even inling.
	struct CodeGeneration {

		const char *endl = "\n";
		struct OutputFile {

			std::vector<int> scopes;
			std::string out;
			int last = 0;
			template<typename T>
			OutputFile& operator<<( const T &v ) { 
				if (scopes.back() == 0) { scopes.back() = 1; out += std::string(4 * ( scopes.size() - 1 ) , ' ') + "{\n"; last = '\n'; }
				for (auto &c : (std::ostringstream() << v).str() ) {
					if ( last == '\n' ) out += std::string( 4 * scopes.size(), ' ' );
					out.push_back(c);
					last = c;
				}
				return *this;
			}
			void start_scope() { scopes.push_back(0); }
			void end_scope() { 
				if (scopes.back()) 
					out += std::string(4 * ( scopes.size() - 1 ) , ' ') + "}\n";
				scopes.pop_back();
			}

		};

		struct OutputState {

			std::string prefix;
			std::string current_code_unit = "base";
			std::map<std::string, OutputFile> code_units;
		};

		OutputState state;

		std::string type_to_c(SyntaxTree::SP ast) {

			if (ast.id() == "type_name") {
				if ( ast->children.size() != 1 ) {
					Log(ERROR) << "More than 1 children?";
				}
				return type_to_c( ast[0] );	
			}

			if (ast.id()=="void") return "void";
			if (ast.id()=="uint8") return "uint8_t";
			if (ast.id()=="uint16") return "uint16_t";
			if (ast.id()=="int8") return "int8_t";
			if (ast.id()=="int16") return "int16_t";
			

			std::string ret = ast.id();


			return ret;

		}	
		
		std::map<std::string, std::function<void(SyntaxTree::SP &)>> processors = {

			{"included_scope", [&](SyntaxTree::SP &ast) { process(ast[0]); }},

			{"namespace", [&](SyntaxTree::SP &ast) { 
				
				auto old_prefix = state.prefix;
				state.prefix += ast[0]->first->literal + "_";
				std::cout << state.prefix << std::endl;
				process(ast[1]); 
				state.prefix = old_prefix;
			}},

			{"type_declaration", [&](SyntaxTree::SP &) { }},

			{"function_definition_with_implicit_type", [&](SyntaxTree::SP &) { }},

	/* FUNCTION CODE IS ONLY GENERATED LAZY, SO WE KNOW WHICH KIND OF POINTERS ARE NEEDED AND IF THEY NEED AN INDEPENDENT MODULE OR NOT.
				auto old_code_unit = state.current_code_unit;
				
				auto function_name = ast[0][2].literal();

				state.current_code_unit = state.prefix + function_name;

				auto &header = state.code_units["declarations.h"];

				auto function_type = ast[0][0][0];

				Log(INFO) << function_name;

				header << "extern " << type_to_c(function_type) << " " << function_name << "(";
				auto &args = ast[0][1]->children;
				for (auto &arg : args) {
					if ( &arg != &args.front() ) header << ", ";
					header << type_to_c(arg[0]);
				}
				header << ")" << endl;

				//process(ast[0][3]); 

				state.current_code_unit = old_code_unit;			
			}},*/

			{"expression", [&](SyntaxTree::SP &asp) { 

				if (asp->children.size() != 1) Log(ERROR) << "Malformed expression.";
				process( asp[0] );
			}},

			{"STRING_LITERAL", [&](SyntaxTree::SP &asp) { 

				auto &oss = state.code_units[state.current_code_unit];
				{
					bool good = asp.literal().size()<40;
					for (uint c : asp.literal()) 
						if (c<32 or c>127) 
							good = false;
					if (good)
						oss << "\\\\ STRING LITERAL: \"" << asp.literal() << "\"\n";
				}

				oss << "const uint8_t " << asp->generated_id << "[] = { ";
				for (uint c : asp.literal()) 
					oss << c << ", ";
				oss << "};" << endl;
			}},

			{"translation_unit", [&](SyntaxTree::SP &ast) { 

				auto &oss = state.code_units[state.current_code_unit];

				oss.start_scope();
				
				for (auto &c : ast->children)
					process( c );
		
				oss.end_scope();
			}},

			{"function_call", [&](SyntaxTree::SP &ast) { 

				auto &oss = state.code_units[state.current_code_unit];

				NamespacedIdentifier function_name(ast->children[0]);
				auto funtion_declaration_ast = function_name.resolve(ast);

	//			std::cout << funtion_ast->type->to_string();

				auto &function_call_args = ast[1]->children;
				auto &function_declaration_args = funtion_declaration_ast->type[0][1]->children;


				if ( function_call_args.size() != function_declaration_args.size() )
					Log(ERROR) << "Wrong number of arguments";

				for (auto &arg : function_call_args) {

					std::cout << arg.to_string();
					process( arg );
				}

	/*			auto return_type = funtion_ast->type[0][0][0][0];
				if (return_type.id() != "void") {
					oss << type_to_c(return_type) << " " << ast->generated_id << " = ";
				}*/

				oss << funtion_declaration_ast->generated_id << "( " ;
				for ( auto &a : function_call_args ) {

					if ( &a != &function_call_args.front() ) oss << ", ";
					oss << a->generated_id;
				}
				oss << " );" << endl;
			}},
		};

		void process(SyntaxTree::SP &ast) {

			std::string symbol = ast.id();

			if (processors.count(symbol)) {
				
				processors[symbol](ast);

			} else {

				Log(ERROR) << symbol << " does not have a code generator (yet)";
			
			}
		}

	};

}


void generate_code( std::string source_file_name ) {

	SourceFile &source_file = SourceFile::Manager::get(source_file_name);

	SyntaxTree::SP main_syntax_tree = std::make_shared<SyntaxTree>( source_file );
	

	if (main_syntax_tree.id() != "translation_unit") 
		Log(ERROR) << "Base syntax tree isn't translation unit but: " << main_syntax_tree.id();


	{
		Log(INFO) << "COMPILER PASS: PREPROCESSOR";
		CompilerPass::Preprocessor preprocessor;
		preprocessor.process(main_syntax_tree);
		std::cout << main_syntax_tree.to_string();
	}
	
	{
		Log(INFO) << "COMPILER PASS: SYMBOL IDENTIFICATION";
		CompilerPass::Symbols identify_symbols;
		identify_symbols.process(main_syntax_tree);
		std::cout << main_syntax_tree.to_string();
	}

	{
		Log(INFO) << "COMPILER PASS: TYPE IDENTIFICATION";
		CompilerPass::Types identify_types;
		identify_types.process(main_syntax_tree);
		std::cout << main_syntax_tree.to_string();
	}

	{
		Log(INFO) << "COMPILER PASS: OPTIMIZER";
		CompilerPass::Optimizer optimizer;
		optimizer.process(main_syntax_tree);
		std::cout << main_syntax_tree.to_string();
	}

	{
		Log(INFO) << "COMPILER PASS: CODE GENERATION";
		CompilerPass::CodeGeneration code_generation;
		code_generation.process(main_syntax_tree);
		//std::cout << main_syntax_tree.to_string();

		for (auto &cu : code_generation.state.code_units) {

			Log(INFO) << cu.first;
			std::cout << cu.second.out << std::endl;

		}
	}
}
