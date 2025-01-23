#pragma once

#include <iostream>
#include "../tokenizer/cn_token.h"
#include "../tokenizer/cn_tokenizer.h"
#include "cn_ast.h"
#include "../ir_generator/cn_ir_generator.h"
#include "../error/sys_error.h"
#include "../main.h"

BaseAST* parse(std::vector<Token*>& tokens);
Token* pull_token_and_expect(std::vector<Token*>& tokens, int token_type);
Token* check_token(std::vector<Token*>& tokens);
void appply_member_data(ClassAST* class_ast, std::vector<BaseAST*>& function_asts);
bool is_operand_of_expression(BaseAST* _cur_node);

struct MemberFunctionData {
	unsigned int id;
	std::wstring name, access_modifier, return_type;
	int parameter_count;
};

struct MemberVariableData {
	unsigned int id;
	std::wstring name, access_modifier, type;
	bool is_array = false;
};
