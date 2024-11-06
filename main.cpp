#include "main.h"

int main() {

	Tokenizer* tokenizer = new Tokenizer();
	Parser* parser = new Parser();

	tokenizer->register_file(L"var x: int = 10 - 5;");

	std::vector<Token*> tokens = tokenizer->tokenize();
	BaseAST* ast = parser->parse_tokens(tokens);

	return 0;
}