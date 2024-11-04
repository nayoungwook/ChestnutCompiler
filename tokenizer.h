#pragma once

#include "token.h"

#include <iostream>
#include <vector>

wchar_t get_char(std::wstring& line, int& _i);
std::vector<Token*> tokenize(std::wstring line, int line_number);
std::wstring get_number_literal(std::wstring& identifier, std::wstring& line, int& i);