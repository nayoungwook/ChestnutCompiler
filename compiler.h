#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "parser/cn_ast.h"
#include "main.h"
#include "compiler.h"

void compile_imported_file(std::string const& import_name);
void create_ir_files();