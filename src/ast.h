#pragma once
#include <tokenizer.h>
#include <grammar.h>

struct SyntaxTree {
private:
	std::string to_string(std::string) const;
public:


	///////////////////////////////////////////////////////////////////////////
	// SP SyntaxTree is generally worked as a shared_ptr.
	// All methods and accessors work directly on the shared_ptr.
	struct SP : std::shared_ptr<SyntaxTree> {
		SP() {}
		SP(const std::shared_ptr<SyntaxTree> &s) : std::shared_ptr<SyntaxTree>(s) {}

		SyntaxTree::SP &operator[](size_t i) { 
			auto &ast = *this;
			if ( ast->children.size() > i ) return ast->children[i]; 
			Log(ERROR) << "Children " << i << " not found in " << id() << ". \n" << show_source(); throw;
		}

		SyntaxTree::SP &operator[](std::string s) { 
			auto &ast = *this;
			for (auto &c : ast->children) if (c and c.id()==s ) return c; 
			Log(ERROR) << s << " not found in " << id() << ". \n" << show_source(); throw;
		}

		std::string id() const { return get()->component.id(); }
		std::string literal() const {return get()->first->literal; }

		struct GLog {
			Log log;
			const SyntaxTree::SP &ast;
			GLog(LogLevel l, const SyntaxTree::SP &ast_) : log(l), ast(ast_) {}
			template<typename T> GLog &operator<<(const T &v) { log << v; return *this; }
			~GLog() { log << ast.show_source(); }
		};

		GLog log(LogLevel l) const { return GLog(l, *this); };
		std::string to_string(std::string prefix = "") const { return this->to_string(prefix); };
		std::string show_source() const;
	};

	using TI = std::vector<Token>::const_iterator;
	
	TI first, last;

	Grammar::Symbol::Component component;

	SP old;
	SP parent;
	std::vector<SP> children;

	bool is_empty = false;
	
	SyntaxTree(TI _first, Grammar::Symbol::Component _component, SP _parent) : first(_first), last(++ _first), component(_component), parent(_parent) {}

	SyntaxTree(SourceFile &file);
	
	// filled during code generation:
	std::map<std::string, SyntaxTree::SP> symbols;
	std::map<std::string, SyntaxTree::SP> attributes;
	SyntaxTree::SP type; 
	std::string c_type;
	std::string generated_id = [](){static uint64_t id = 10000000; return "____mc" + std::to_string(id++) + "_";}();
};

