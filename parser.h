#pragma once

#include "ast.h"
#include "token.h"

class Parser {
private:
	Token* lookahead(std::vector<Token*>& tokens, token_type expected_type);
	Token* check_token(std::vector<Token*>& tokens);
	BaseAST* create_expression_ast(std::vector<Token*>& tokens);
	std::vector<BaseAST*> to_postfix(std::vector<BaseAST*>& nodes);
	int get_nodes_priority(const BaseAST* node);
public:
	BaseAST* parse_tokens(std::vector<Token*>& tokens);
};