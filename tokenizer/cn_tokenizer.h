#pragma once
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "cn_token.h"

static char cur_char = ' ', next_char = ' ';

char get_char(std::string& line, int& _i);

std::vector<Token*> tokenize(std::string line, int line_number);
std::string  get_number_literal(std::string& identifier, std::string& line, int& i);
