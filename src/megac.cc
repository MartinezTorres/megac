#include "parser.h"
#include "file.h"
#include "generator.h"

#include <boost/program_options.hpp>


int main(int argc, char *argv[]) {
		
	// PARSING PROGRAM OPTIONS
	namespace po = boost::program_options;

	po::options_description pod("MegaC Compiler");
	pod.add_options() 
		("help,h", "produce this help message")
		("log,l", po::value<int>()->default_value(0), "set log level")
		("source_file,i", po::value<std::string>(), "Input source files");

	po::positional_options_description p;
	p.add("source_file", 1);
	
	po::variables_map pom;
	po::store( po::command_line_parser( argc, argv).options(pod).positional(p).run(), pom);
	po::notify(pom);

	if (pom.count("help") or pom.count("source_file") == 0) {
		std::cout << "Usage:" << std::endl <<  pod << "\n";
		return 0;
	}
	
	Log::report_level() = static_cast<LogLevel>(pom["log"].as<int>());

	std::string source_file_name = pom["source_file"].as<std::string>();

	SourceFile &source_file = SourceFile::Manager::get(source_file_name);

	SyntaxTree main_syntax_tree( source_file );
	
	std::cout << main_syntax_tree.to_string();
	
	generate_code(main_syntax_tree);
	
	
	
/*	std::vector<Token> = tokenize(
	std::map<std::string, std::shared_ptr<const SourceFile>> files;
	std::map<std::string, > tokens;
	
	files[source_file] = std::make_shared<const SourceFile>(source_file);
	
	tokens[source_file] = tokenize(files[source_file]);*/
	

	return 0;
}
