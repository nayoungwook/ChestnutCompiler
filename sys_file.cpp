#include "sys_file.h"

std::vector<std::string> get_file(std::string& file_path) {

	std::string current_path;
	std::wstring wcurrent_dir = get_current_directory();
	current_path.assign(wcurrent_dir.begin(), wcurrent_dir.end());

	file_path = current_path + "\\" + file_path;

	std::ifstream openFile(file_path.data());
	std::vector<std::string> result;

	if (openFile.is_open()) {
		std::string line;
		while (getline(openFile, line)) {
			result.push_back(line);
		}
		openFile.close();
	}
	else {
		std::wstring w_path;
		w_path.assign(file_path.begin(), file_path.end());

		CHESTNUT_THROW_ERROR(L"Failed to find file at : " + w_path, "FAILED_TO_FIND_FILE", "001", 0);
	}
	return result;
}

void write_file(std::string file_path, std::string content) {
	std::vector<std::string> result;

	std::string current_path;
	std::wstring wcurrent_dir = get_current_directory();
	current_path.assign(wcurrent_dir.begin(), wcurrent_dir.end());

	file_path = current_path + "\\" + file_path;
	std::cout << file_path << std::endl;

	std::ofstream writeFile(file_path.data());
	if (writeFile.is_open()) {
		writeFile << content;
		writeFile.close();
	}
}

std::wstring get_current_directory() {
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}