#include "sys_logger.h"

void CHESTNUT_LOG(const std::wstring& content, log_level c) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	const auto now = std::chrono::system_clock::now();
	const std::time_t t_c = std::chrono::system_clock::to_time_t(now);

	SetConsoleTextAttribute(hConsole, (int)c);
	char* time_str;
	char t[256];

	ctime_s(t, sizeof(t), &t_c);
	std::string log_level = "";

	switch (c) {
	case log_level::log_okay:
		log_level = "OKAY";
		break;
	case log_level::log_error:
		log_level = "ERROR";
		break;
	case log_level::log_default:
		log_level = "DEFAULT";
		break;
	case log_level::log_success:
		log_level = "SUCCESS!";
		break;
	case log_level::log_warn:
		log_level = "WARNING";
		break;
	}

	std::cout << "[CHESTNUT : " + log_level + "]> ";
	std::wcout << content << '\n';
	SetConsoleTextAttribute(hConsole, 07);
}