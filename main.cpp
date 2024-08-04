#include "main.h"

int main(int argc, char* args[]) {

	declare_builtin_functions();

	if (argc == 1) {

		CHESTNUT_LOG(L"With no arguments, the process is on the \'debug mode\'", log_level::log_warn);
		CHESTNUT_LOG(L"If it is not your intention, please type \'chestnut -help.\'", log_level::log_warn);

		std::string file_name = "main.cn";

		std::vector<std::string> _file = get_file(file_name);

		std::vector<Token*> tokens;
		for (int i = 0; i < _file.size(); i++) {
			std::vector<Token*> lines;
			try {
				lines = tokenize(_file[i], 1 + i);
			}
			catch (int e) {
				std::wstring w_line;
				w_line.assign(_file[i].begin(), _file[i].end());
				CHESTNUT_THROW_ERROR(L"Unexpected error at : " + w_line, "UNEXPECTED_ERROR", "002", i);
			}

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

		CHESTNUT_LOG(L"main.cn compiled successfully.", log_level::log_success);

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