#include "main.h"

	int main(int argc, char* args[]) {

	declare_builtin_functions();

	if (argc == 1) {

		std::string file_name = "main.cn";

		std::vector<std::string> _file = get_file(file_name);

		std::vector<Token*> tokens;
		for (int i = 0; i < _file.size(); i++) {
			std::vector<Token*> lines = tokenize(_file[i], i);

			for (Token* tkn : lines) {
				tokens.push_back(tkn);
			}
		}

		std::vector<BaseAST*> parsed_asts;

		std::string result = "";
		while (!tokens.empty()) {
			if (tokens.size() == 0) continue;

			parsed_asts.push_back(parse(tokens));

		}

		for (BaseAST* ast : parsed_asts)
			result += create_ir(ast, 0) + "\n";

		std::cout << "<main.cn compiled>" << std::endl;
		std::cout << result;
		write_file("main.cir", result);

		/*
		std::cout <<
			"ChestnutCompiler (indev version).\n"
			"please compile source code with : chestnutc -compile \"file name\"";
		*/
	}

	return 0;
}