#pragma once

#include "token.h"

#include <iostream>
#include <vector>

class Tokenizer {
public:
	std::vector<Token*> tokenize();
	void register_file(std::wstring const line);
private:
	std::vector<Token*> tokenize_line(std::wstring line, int line_number);
	std::vector<std::wstring> file;
	wchar_t get_char(std::wstring& line, int& _i);
	std::wstring get_number_literal(std::wstring& identifier, std::wstring& line, int& i);
};