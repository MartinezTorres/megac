#pragma once

#include <memory>
#include <vector>

#include <file.h>

////////////////////////////////////////////////////////////////////////
// TOKENIZER

struct Token {

	enum Type {
		IDENTIFIER,
		PUNCTUATOR,
		STRING_LITERAL,
		NUMERIC
	} type;
	
	// Content
	uint64_t val = 0;
	std::string literal;
	
	// Token location and length
	NormalizedSourcePtr begin_ptr, end_ptr;
	
	bool has_space;      // True if this token follows a space character
	bool start_of_line;  // True if this token is at beginning of line
	
	std::string to_string() const;
	std::string show_source() const;
	
	Token(Type _type, uint64_t _val, NormalizedSourcePtr _begin_ptr, NormalizedSourcePtr _end_ptr, bool _has_space, bool _start_of_line ) 
		: type(_type), val(_val), begin_ptr(_begin_ptr), end_ptr(_end_ptr), has_space(_has_space), start_of_line(_start_of_line)  {}

	Token(Type _type, std::string _literal, NormalizedSourcePtr _begin_ptr, NormalizedSourcePtr _end_ptr, bool _has_space, bool _start_of_line ) 
		: type(_type), literal(_literal), begin_ptr(_begin_ptr), end_ptr(_end_ptr),  has_space(_has_space), start_of_line(_start_of_line)  {}
		
	friend std::ostream& operator<<(std::ostream& os, const Token &t) { os << t.to_string(); return os; }
};

std::vector<Token> &tokenize(SourceFile &file);
