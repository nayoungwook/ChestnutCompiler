#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ostream>
#include <ctime>
#include <Windows.h>
#include <chrono>
#include "logger/sys_logger.h"
#include "error/sys_error.h"

std::vector<std::string> get_file(std::string& file_path);
void write_file(std::string file_path, std::string content);
std::wstring get_current_directory();