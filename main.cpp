#include "main.h"

std::vector<BaseAST*> compile_file(std::wstring const& file_name) {

	std::wstring cn_file_path = file_name + L".cn";
	std::wstring w_file_name = std::wstring(cn_file_path.begin(), cn_file_path.end());
	std::vector<std::wstring> _file = get_file(w_file_name);

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

	std::wstring result = L"";
	while (!tokens.empty()) {
		if (tokens.size() == 0) continue;

		parsed_asts.push_back(parse(tokens));
	}

	CHESTNUT_LOG(L"File successfully compiled : " + std::wstring(file_name.begin(), file_name.end()), log_level::log_success);

	return parsed_asts;
}

std::wstring create_ir(std::vector<BaseAST*>& parsed_asts) {
	std::wstring result;

	for (BaseAST* ast : parsed_asts)
		result += create_ir(ast, 0) + L"\n";

	return result;
}

int main(int argc, char* args[]) {

	declare_builtin_functions();
	declare_builtin_variables();

	if (argc == 1) {
		CHESTNUT_LOG(L"With no arguments, the process is on the \'debug mode\'", log_level::log_warn);
		CHESTNUT_LOG(L"If it was not your intention, please type \'chestnut -help.\' to get help.", log_level::log_warn);

		compile_imported_file(L"main");
	}
	else if (argc == 2) {
		if (!strcmp("-credits-fuck", args[1])) {
			std::wcout << L"Chsetnut Programming language" << std::endl;
			std::wcout << L"ÄÚµå ½Ñ °³»õ³¢ - ³ª¿µ¿í" << std::endl;
			std::wcout << L"ÈÆ¼ö µÐ ¾¾¹ß·Ò - ±èÁØ¼­" << std::endl;
			std::wcout << L"±×¸² ±×¸° ¹ÌÄ£³ð - ¹Ú¼¼Çö" << std::endl;
		}
		else if (!strcmp("-credits", args[1])) {
			std::wcout << L"ÇÒ¸Ó´Ï¿¡°Ô" << std::endl;

			std::wcout << L"Chsetnut Programming language" << std::endl;
			std::wcout << L"Developed by NaYoungWook" << std::endl;
			std::wcout << L"Special thanks to KimJunSeo" << std::endl;
			std::wcout << L"Logo pic by ParkSehyun" << std::endl;
		}
	}
	else if (argc == 3) {
		if (!strcmp("-compile", args[1])) {
			std::string file_name = args[2];

			compile_imported_file(std::wstring(file_name.begin(), file_name.end()));
		}
		else if (!strcmp("-help", args[1])) {
			CHESTNUT_LOG(L"With no arguments, the process is on the \'debug mode\'", log_level::log_warn);

		}
	}

	create_ir_files();

	return 0;
}