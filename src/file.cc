#include <file.h>

NormalizedSourcePtr& NormalizedSourcePtr::operator++() { 
	
	if ( c=='\n' ) {
		
		line++;
		start_line = it;
	}
	
	// canonicalize newline
	if ( it.starts_with("\r\n") ) { c = '\n'; it+=2; return *this; }
	if ( it.starts_with("\r"  ) ) { c = '\n'; it++;  return *this; }
	
	// remove backslash newline
	if ( it.starts_with("\\\n"  ) ) { c='\n'; it+=2; return ++(*this); }
	if ( it.starts_with("\\\r"  ) ) { c='\n'; it+=2; return ++(*this); }
	if ( it.starts_with("\\\r\n") ) { c='\n'; it+=3; return ++(*this); }

	c = *it++;
	return *this; 
}

std::string NormalizedSourcePtr::show_source( const NormalizedSourcePtr &begin, const NormalizedSourcePtr &end ) {

//	constexpr auto RESET = "\x1b[0m";
//	constexpr auto BLUE  = "\x1b[1;34m";
//	constexpr auto RED   = "\x1b[1;31m";
//	constexpr auto GREEN = "\x1b[1;32m";
	
	auto end_it = end.it;
	if (begin.source_file_ptr->path != end.source_file_ptr->path) {
		Log(ERROR_NOTHROW) << " visualizing a file range from two different files.";
		end_it = begin.it;
	}

	if (begin.line > end.line) {
		Log(ERROR_NOTHROW) << " visualizing a file range that ends earlier than finishes.";
		end_it = begin.it;
	}

	if (begin.line != end.line) 
		end_it = begin.it;

	std::ostringstream os;

	//os << RESET << begin.source_file_ptr->path << ":" << begin.line << ":" << std::endl;
	os << std::fixed << std::setw( 5 ) << begin.line << " | ";
	for (auto i = begin.start_line; i and *i != '\n'; i++ ) os.put(*i);
	os << std::endl << "      |";
	for (auto i = begin.start_line; i and (i != begin.it); i++ ) os.put(' ');
	for (auto i = begin.it; i and (i != end_it); i++ ) os.put('^');
	if (begin.it == end_it) os << "^";
	os << " ";
	
	return os.str();
}

