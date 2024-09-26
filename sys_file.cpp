#include "sys_file.h"

std::vector<std::wstring> get_file(std::wstring& file_path) {

	std::wstring current_path;
	std::wstring wcurrent_dir = get_current_directory();
	current_path.assign(wcurrent_dir.begin(), wcurrent_dir.end());

	file_path = std::wstring(current_path.begin(), current_path.end()) + L"\\" + file_path;

	std::wifstream openFile(file_path.data());
	std::vector<std::wstring> result;

	if (openFile.is_open()) {
		std::wstring line;
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

void write_file(std::wstring file_path, std::wstring content) {
	std::vector<std::wstring> result;

	std::wstring current_path;
	std::wstring wcurrent_dir = get_current_directory();
	current_path.assign(wcurrent_dir.begin(), wcurrent_dir.end());

	file_path = current_path + L"\\" + file_path;

	std::wofstream writeFile(file_path.data());
	if (writeFile.is_open()) {
		writeFile << content;
		writeFile.close();
	}
}

std::wstring get_current_directory() {
	TCHAR buffer[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, buffer);
	return std::wstring(buffer);
}