#pragma once

#include <iostream>
#include "../tokenizer/cn_token.h"
#include "../tokenizer/cn_tokenizer.h"
#include "cn_ast.h"


BaseAST* parse(std::vector<Token*>& tokens);
Token* pull_token_and_expect(std::vector<Token*>& tokens, int token_type);
Token* check_token(std::vector<Token*>& tokens);
