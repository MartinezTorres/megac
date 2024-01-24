#include "megac.h"


/*static void print_tokens(Token *tok) {
  FILE *out = open_file(opt_o ? opt_o : "-");

  int line = 1;
  for (; tok->kind != TK_EOF; tok = tok->next) {
    if (line > 1 && tok->at_bol)
      fprintf(out, "\n");
    if (tok->has_space && !tok->at_bol)
      fprintf(out, " ");
    fprintf(out, "%.*s", tok->len, tok->loc);
    line++;
  }
  fprintf(out, "\n");
}*/

int main(int argc, char** argv) {
	
	std::vector<CToken> tokens = tokenize_file(path);
	
	tokens = preprocess(tokens);
	
	if (options.print_preprocessed) {
		
		print_c_tokens(tok);
		return 0;
	}
	
	Ast prog = parse(tokens);
	
	TokenASM = codegen(prog);
	
}
