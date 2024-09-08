#include "main.h"

std::vector<BaseAST*> compile_file(std::string const& file_name) {

	std::string cn_file_path = file_name + ".cn";
	std::vector<std::string> _file = get_file(cn_file_path);

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

	CHESTNUT_LOG(L"File successfully compiled : " + std::wstring(file_name.begin(), file_name.end()), log_level::log_success);

	return parsed_asts;
}

std::string create_ir(std::vector<BaseAST*>& parsed_asts) {
	std::string result;

	for (BaseAST* ast : parsed_asts)
		result += create_ir(ast, 0) + "\n";

	return result;
}

int main(int argc, char* args[]) {

	declare_builtin_functions();
	declare_builtin_variables();

	if (argc == 1) {
		CHESTNUT_LOG(L"With no arguments, the process is on the \'debug mode\'", log_level::log_warn);
		CHESTNUT_LOG(L"If it was not your intention, please type \'chestnut -help.\' to get help.", log_level::log_warn);

		compile_imported_file("main");
	}
	else if (argc == 3) {
		if (!strcmp("-compile", args[1])) {
			std::string file_name = args[2];

			compile_imported_file(file_name);
		}
	}

	create_ir_files();

	return 0;
}