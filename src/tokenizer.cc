#include <tokenizer.h>
#include <file.h>

#include <iterator> 
#include <cstddef> 
#include <optional>

#include <log.h>

std::string Token::to_string() const {

	constexpr auto RESET = "\x1b[0m";

	constexpr auto BLUE  = "\x1b[1;34m";
	constexpr auto RED   = "\x1b[1;31m";
	constexpr auto GREEN = "\x1b[1;32m";
	
	std::string ret;
	
	if (type == Token::NUMERIC) ret = ret + "N[" + BLUE + std::to_string(val) + RESET + "]";
	if (type == Token::STRING_LITERAL) ret = ret + "S[" + BLUE + literal + RESET + "]";
	if (type == Token::PUNCTUATOR) ret = ret + "P[" + GREEN + literal + RESET + "]";
	if (type == Token::IDENTIFIER) ret = ret + "I[" + RED + literal + RESET + "]";
	
	return ret;
}

std::string Token::show_source() const {
	
	return begin_ptr.to_string();
}


namespace Utilities {

	static void printSource( NormalizedSourcePtr p ) {
		
		Log(EXTRA) << "Contents of file: " << p.get_file().path;

		size_t line = 0;
		char last = '\n';
		while (p) {
			if (p.get_line() != line) {
				line = p.get_line();
				
				if (last=='\n')
					std::cout << std::fixed << std::setw( 5 ) << line << " | ";
			}
			std::cout << char(last = *p++);
		}
		std::cout << std::endl;
	}

	static void printTokens( std::vector<Token> tokens ) {

		Log(EXTRA) << "Start list of tokens";

		size_t line = 0;
		for (auto &t : tokens) {
			
			if (t.begin_ptr.get_line() != line) {
				if (line) std::cout << std::endl;
				line = t.begin_ptr.get_line();
				std::cout << std::fixed << std::setw( 5 ) << line << " | ";
			}
			if (t.has_space) std::cout << " ";
			std::cout << t;
		}
		std::cout << std::endl;
		Log(EXTRA) << "End list of tokens";
	}
}

namespace TokenParsingUtilities {
	
	static char get_escaped_char( NormalizedSourcePtr &p ) {
		
		if ( *p != '\\' ) return *p++;
		
		++p;
		// Unscaping char
		if ('0' <= *p && *p <= '7') { // Read an octal number.
			
			int c = *p++ - '0';
			
			if ('0' <= *p && *p <= '7') {
			
				c = (c << 3) + (*p++ - '0');
				
				if ('0' <= *p && *p <= '7')
					c = (c << 3) + (*p++ - '0');
			}
			
			return c;
		} 
		
		if (*p == 'x') { // Read a hexadecimal number.

			++p;

			if ( not std::isxdigit( *p ) )
				Log(ERROR) << p << "invalid hex escape sequence";

			int c = 0;
			for (; std::isxdigit( *p ); p++) {
				
				int v = 0;
				if ('0' <= *p and *p <= '9') v = *p - '0';
				if ('a' <= *p and *p <= 'f') v = *p - 'a' + 10;
				if ('A' <= *p and *p <= 'F') v = *p - 'A' + 10;

				c = (c << 4) + v;
			}
			
			return c;
		} 

		char c = *p++;
		if (c == 'a') return '\a';
		if (c == 'b') return '\b';
		if (c == 't') return '\t';
		if (c == 'n') return '\n';
		if (c == 'v') return '\v';
		if (c == 'f') return '\f';
		if (c == 'r') return '\r';

		return c;
	}	
}

namespace TokenParsers {
	
	static std::optional<Token> numeric( NormalizedSourcePtr p, bool has_space, bool start_of_line) { 

		if ( not std::isdigit(p[0]) ) return std::nullopt;

		auto start_ptr = p;
		
		uint64_t val = [&p]() -> uint64_t { 
		
			uint64_t n = 0;
			
			if (p[0] != '0') { 
				
				char c;
				while ( std::isdigit( c = *p ) ) {
					p++;
					n = 10 * n + c - '0';
				}
					
				return n;
			}
			
			if (p[1] == 'x') {
				
				p+=2;
				
				if ( not std::isxdigit( *p ) ) Log(ERROR) << p << "malformed hexadecimal constant";
				
				char c;
				while ( std::isxdigit( c = *p ) ) {
					
					p++;
					if ('0' <= c and c <= '9') n = 16 * n + c - '0';
					if ('a' <= c and c <= 'f') n = 16 * n + c - 'a' + 10;
					if ('A' <= c and c <= 'F') n = 16 * n + c - 'A' + 10;
				}
				
				return n;
			}

			if (p[1] == 'b') {
				
				p+=2;
				
				if ( *p != '0' and *p != '1' ) Log(ERROR) << p << "malformed binary constant";
				
				while ( *p == '0' or *p == '1' ) {
					
					n = 2 * n + *p - '0';
					p++;
				}
				
				return n;
			}
			
			++p;
			if ( std::isxdigit( *p ) ) Log(ERROR) << p << "malformed number constant";
			
			return 0;
		}();
		
		auto end_ptr = p;
		
		return std::make_optional<Token>(Token::NUMERIC, val, start_ptr, end_ptr, has_space, start_of_line);
	}

	static std::optional<Token> string( NormalizedSourcePtr p, bool has_space, bool start_of_line) {
		
		if ( *p != '"' ) return std::nullopt;
		
		auto start_ptr = p++;

		for (;;) {

			if ( *p == '"' ) break;

			if ( *p == '\n' or *p == '\0')
				Log(ERROR) << p << "unclosed string literal";
				
			if ( *p == '\\' ) ++p;
			
			++p;
		}

		auto end_ptr = p;
		
		p = start_ptr; ++p;

		// Unscaping literal
		std::string literal;
		while (p != end_ptr) 
			literal.push_back( TokenParsingUtilities::get_escaped_char(p) );
		
		end_ptr++;

		return std::make_optional<Token>(Token::STRING_LITERAL, literal, start_ptr, end_ptr, has_space, start_of_line);
	}

	static std::optional<Token> character(  NormalizedSourcePtr p, bool has_space, bool start_of_line ) { 

		if ( *p != '\'' ) return std::nullopt;

		auto start_ptr = p++;

		uint64_t val = TokenParsingUtilities::get_escaped_char(p);
		
		if ( *p != '\'')
				Log(ERROR) << p << "unclosed character literal";

		auto end_ptr = ++p;
		
		return std::make_optional<Token>(Token::NUMERIC, val, start_ptr, end_ptr, has_space, start_of_line);
	}

	static std::optional<Token> identifier(  NormalizedSourcePtr p, bool has_space, bool start_of_line ) { 
		
		if ( p[0] != '_' and not std::isalpha(p[0]) ) return std::nullopt;
		
		auto start_ptr = p;
		
		std::string literal;
		
		while ( std::isalnum(*p) or *p=='_' ) 
			literal.push_back(*p++);
			
		auto end_ptr = p;

		return std::make_optional<Token>(Token::IDENTIFIER, literal, start_ptr, end_ptr, has_space, start_of_line);
	}

	static std::optional<Token> punctuator(  NormalizedSourcePtr p, bool has_space, bool start_of_line ) { 
		
		
		for (auto c : { "<<=", ">>=", "...", ":=", "==", "!=", "<=", ">=", "->", "+=", "-=", "*=", "/=", "++", "--", "%=", "&=", "|=", "^=", "&&", "||", "<<", ">>", "##", "::" } ) {
			
			if (p.starts_with(c)) {
				
				auto start_ptr = p;
			
				std::string literal = c;
				p += literal.size();
					
				auto end_ptr = p;
				
				return std::make_optional<Token>(Token::PUNCTUATOR, literal, start_ptr, end_ptr, has_space, start_of_line);
			}
		}
		
		if ( std::ispunct( *p ) ) {
			
			auto start_ptr = p;
		
			std::string literal;
			literal.push_back(*p++);
				
			auto end_ptr = p;
			
			return std::make_optional<Token>(Token::PUNCTUATOR, literal, start_ptr, end_ptr, has_space, start_of_line);
		}
		
		return std::nullopt;
	}
}

std::vector<Token> &tokenize(SourceFile &file) {

	NormalizedSourcePtr p(file);

	// UTF-8 texts may start with a 3-byte "BOM" marker sequence.
	// If exists, just skip them because they are useless bytes.
	// (It is actually not recommended to add BOM markers to UTF-8
	// texts, but it's not uncommon particularly on Windows.)
	if ( p.starts_with("\xef\xbb\xbf") ) p += 3;
	
	if (Log::report_level() == EXTRA) Utilities::printSource(p);
	
	file.tokens = std::make_shared<std::vector<Token>>();
	std::vector<Token> &tokens = *file.tokens;

	bool start_of_line = true;
	bool has_space = false;

	while (p) {
		
		if ( p.starts_with( "//" ) ) { // Skip line comments.
			
			while ( *p != '\n' ) ++p;
			has_space = true;
			continue;
		}

		if ( p.starts_with( "/*" ) ) { // Skip block comments.
			
			p += 2;
			while ( p and not p.starts_with( "*/" ) ) {
				
				if ( p.starts_with( "/*" ) ) Log(WARNING) << "\"/*\" within block comment" << p;				
				p++;
			}
			
			if ( not p ) Log(ERROR) << "unclosed block comment" << p;
			
			p += 2;
			has_space = true;
			continue;
		}
	
		if ( *p == '\n' ) { // Skip newline.

			p++;
			start_of_line = true;
			has_space = false;
			continue;
		}

		if ( std::isspace(*p) ) { // Skip whitespace characters.

			p++;
			has_space = true;
			continue;
		}

		std::optional<Token> t;
		if ( not t.has_value() ) t = TokenParsers::numeric(p, has_space, start_of_line);
		if ( not t.has_value() ) t = TokenParsers::string(p, has_space, start_of_line);
		if ( not t.has_value() ) t = TokenParsers::character(p, has_space, start_of_line);
		if ( not t.has_value() ) t = TokenParsers::identifier(p, has_space, start_of_line);
		if ( not t.has_value() ) t = TokenParsers::punctuator(p, has_space, start_of_line);
		if ( not t.has_value() ) Log(ERROR) << "invalid token" << p;

		has_space = false;
		start_of_line = false;

		tokens.push_back(t.value());
		p = t.value().end_ptr;
	}
	
	if (Log::report_level() == EXTRA) Utilities::printTokens (tokens);
	
	return tokens;
}
