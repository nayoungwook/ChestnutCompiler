#include "sys_error.h"

void CHESTNUT_THROW_ERROR(std::wstring const& message,
	std::string const& error_type, std::string const& error_code, int line_number) {
	std::cout << "=====================================\n";
	std::wstring w_error_type, w_error_code;

	w_error_type.assign(error_type.begin(), error_type.end());
	w_error_code.assign(error_code.begin(), error_code.end());

	std::wstring w_line_number;
	std::string line_number_str = std::to_string(line_number);

	w_line_number.assign(line_number_str.begin(), line_number_str.end());

	CHESTNUT_LOG(L"Woops! Error occured at line number : " + w_line_number, log_level::log_error);
	CHESTNUT_LOG(L"Error : " + w_error_type + L" | Error code : " + w_error_code, log_level::log_error);
	CHESTNUT_LOG(L"\"" + message + L"\"", log_level::log_error);
	std::cout << "=====================================\n";

	exit(EXIT_FAILURE);
}