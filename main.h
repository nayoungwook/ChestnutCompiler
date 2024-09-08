#pragma once

#include <iostream>
#include <unordered_set>

#include "parser/cn_parser.h"
#include "tokenizer/cn_token.h"
#include "tokenizer/cn_tokenizer.h"
#include "parser/cn_ast.h"
#include "sys_file.h"
#include "ir_generator/cn_ir_generator.h"
#include "compiler.h"

std::vector<BaseAST*> compile_file(std::string const& file_name);
std::string create_ir(std::vector<BaseAST*>& parsed_asts);
