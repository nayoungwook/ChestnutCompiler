#pragma once

#include <iostream>
#include <string>
#include <locale>
#include <chrono>
#include <Windows.h>

enum class log_level {
	log_okay = 3,
	log_error = 4,
	log_success = 11,
	log_default = 7,
	log_warn = 12,
};

void CHESTNUT_LOG(const std::wstring& content, log_level c);