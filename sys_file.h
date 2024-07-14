#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ostream>
#include <ctime>
#include <Windows.h>
#include <chrono>

std::vector<std::string> get_file(std::string& file_path);
void write_file(std::string file_path, std::string content);
std::wstring get_current_directory();