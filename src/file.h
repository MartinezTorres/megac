#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <iomanip>

#include "log.h"

struct Token;
struct SyntaxTree;

struct SourceFile {
	
	std::string path;
	std::shared_ptr<std::string> source;
	std::shared_ptr<std::vector<Token>> tokens;
	
	SourceFile(std::string _path) : path(_path) {
			
		try {

			Log(INFO) << "Opening file: " << path;

			std::ifstream inputFile(path, std::ios::binary);
			std::ostringstream fileContent;
			fileContent << inputFile.rdbuf();
			source = std::make_shared<std::string>(fileContent.str());
			
		} catch (std::exception const&  e) {
		
			Log(ERROR) << ASSERT_INFO << e.what();
		}

		if (not source) 
			Log(ERROR) << " error reading source file";
	}

	SourceFile(std::string name, const std::string_view &content) : path(name), source(std::make_shared<std::string>(content)) {}

	class Manager {
		
		static std::map<std::string, SourceFile> &storage() {
			static std::map<std::string, SourceFile> st;
			return st;
		}
	public:
		
		static SourceFile &get( std::string path ) {
			
			if (storage().count(path) == 0)	
				storage().emplace(path, SourceFile(path) );
			
			return storage().at(path);
		}

		static SourceFile &get( std::string path, const std::string_view &content ) {
			
			if (storage().count(path) == 0)	
				storage().emplace(path, SourceFile(path, content) );
			
			return storage().at(path);
		}
	};
};



class NormalizedSourcePtr {

	struct Utilities {
		template<typename T>
		inline static bool starts_with(T ptr, std::string s) { 
			for (auto c : s) 
				if ( *ptr++ != c ) return false;
			return true;
		}
	};

	class SourcePtr {
		std::string *src;
		std::string::iterator it;

	public:
		SourcePtr(const SourceFile &source_file) : src(source_file.source.get()), it(src->begin()) {} 

		bool starts_with(std::string s) const { return Utilities::starts_with(*this, s); }

		char operator*() const { if (it == src->end()) return 0; return *it; }

		SourcePtr operator++(int) { auto tmp = *this; ++(*this); return tmp; }
		SourcePtr& operator++() { if ( not (it == src->end()) ) ++it; return *this; }
		
		SourcePtr& operator+=(size_t idx) { while (idx--) ++(*this); return *this; }
		char operator[](size_t idx) { auto tmp = *this; tmp += idx; return *tmp; }

		auto operator<=>(const SourcePtr& o) const = default;
		operator bool() const { return it != src->end(); }
	};

	////////////////////////////////////////////////////////////////////
	// Private members

	char c = 0;
	const SourceFile *source_file_ptr;
	SourcePtr start_line, it;

	size_t line = 1;

public:

	bool starts_with(std::string s) const { return Utilities::starts_with(*this, s); }
	size_t get_line() const { return line; }
	const SourceFile &get_file() const { return *source_file_ptr; }

	NormalizedSourcePtr(const SourceFile &source_file) : 
		source_file_ptr(&source_file), start_line(*source_file_ptr), it(*source_file_ptr) { ++(*this); }

	char operator*() const { return c; }

	NormalizedSourcePtr operator++(int) { auto tmp = *this; ++(*this); return tmp; }
	NormalizedSourcePtr& operator++();
	
	NormalizedSourcePtr& operator+=(size_t idx) { while (idx--) ++(*this); return *this; }
	char operator[](size_t idx) { auto tmp = *this; tmp += idx; return *tmp; }

	auto operator<=>(const NormalizedSourcePtr& o) const = default;
	operator bool() const { return it; }

	std::string to_string() const { return NormalizedSourcePtr::show_source(*this, *this); }
	static std::string show_source(const NormalizedSourcePtr &begin, const NormalizedSourcePtr &end);
	friend std::ostream& operator<<(std::ostream& os, const NormalizedSourcePtr &p) { return os << p.to_string(); }
};
