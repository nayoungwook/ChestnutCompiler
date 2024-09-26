#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <ostream>
#include <ctime>
#include <Windows.h>
#include <chrono>
#include "logger/sys_logger.h"
#include "error/sys_error.h"

std::vector<std::wstring> get_file(std::wstring& file_path);
void write_file(std::wstring file_path, std::wstring content);
std::wstring get_current_directory();