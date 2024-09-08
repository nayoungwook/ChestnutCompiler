#include "compiler.h"

std::unordered_set<std::string> compiled_files;
std::vector<std::pair<std::string, std::vector<BaseAST*>>> compiled_asts;

void compile_imported_file(std::string const& import_name) {

	if (compiled_files.find(import_name) != compiled_files.end()) {
		return;
	}

	compiled_files.insert(import_name);
	std::vector<BaseAST*> ast = compile_file(import_name);
	compiled_asts.push_back(std::make_pair(import_name, ast));
}

void create_ir_files() {
	while (!compiled_asts.empty()) {
		std::pair<std::string, std::vector<BaseAST*>> compiled = compiled_asts[0];
		std::string result_ir = create_ir(compiled.second);
		std::wstring w_file_path;

		w_file_path.assign(compiled.first.begin(), compiled.first.end());
		CHESTNUT_LOG(L"IR successfully created : " + w_file_path, log_level::log_success);
		write_file(compiled.first + ".cir", result_ir);

		compiled_asts.erase(compiled_asts.begin());
	}
}