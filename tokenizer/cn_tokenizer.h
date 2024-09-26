#pragma once
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "cn_token.h"

static wchar_t cur_char = L' ', next_char = L' ';

wchar_t get_char(std::wstring& line, int& _i, int line_number);
bool is_special_characters(const wchar_t& wc);

std::vector<Token*> tokenize(std::wstring line, int line_number);
std::wstring get_number_literal(std::wstring& identifier, std::wstring& line, int& i, int line_number);
