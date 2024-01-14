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

std::string NormalizedSourcePtr::to_string() const {
	
	std::ostringstream os;

	os << source_file_ptr->path << ":" << line << ":" << std::endl;
	os << std::fixed << std::setw( 5 ) << line << " | ";
	for (auto i = start_line; i and *i != '\n'; i++ ) os.put(*i);
	os << std::endl << "      |";
	for (auto i = start_line; i and (i != it); i++ ) os.put(' ');
	os << "^ ";
	
	return os.str();
}

