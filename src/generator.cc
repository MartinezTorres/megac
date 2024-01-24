#include "generator.h"

void generate_code(SyntaxTree ast) {
	
	
	
	if (ast.symbol != "translation_unit") Log(ERROR) << "Base syntax tree isn't translation unit but: " << ast.symbol;
	
	
	
}
