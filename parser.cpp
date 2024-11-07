#include "parser.h"

Token* Parser::lookahead(std::vector<Token*>& tokens, token_type expected_type) {
	if (tokens.empty()) {
		// Evoke Error, there's no tokens.
	}

	Token* tok = tokens[0];
	tokens.erase(tokens.begin());

	if (expected_type == tok_none) {
		return tok;
	}

	if (tok->get_type() != expected_type) {
		// Evoke Error, wrong token type
	}

	return tok;
}

Token* Parser::check_token(std::vector<Token*>& tokens) {
	if (tokens.empty()) {
		// Evoke Error, there's no tokens.
	}

	Token* tok = tokens[0];

	return tok;
}

int Parser::get_nodes_priority(const BaseAST* node) {
	if (node->type == operator_ast) {
		OperatorAST* _oper_node = (OperatorAST*)node;
		if (_oper_node->oper == L"+" || _oper_node->oper == L"-")
			return 1;
		if (_oper_node->oper == L"*" || _oper_node->oper == L"/" ||
			_oper_node->oper == L"^" || _oper_node->oper == L"%")
			return 2;
		if (_oper_node->oper == L"==" || _oper_node->oper == L"!=" ||
			_oper_node->oper == L"<=" || _oper_node->oper == L">=" ||
			_oper_node->oper == L"<" || _oper_node->oper == L">")
			return 0;
		if (_oper_node->oper == L"||" || _oper_node->oper == L"&&")
			return -1;
	}
	return -2;
}

std::vector<BaseAST*> Parser::to_postfix(std::vector<BaseAST*>& nodes) {
	std::vector<BaseAST*> result;
	std::stack<BaseAST*> stk;

	for (int i = 0; i < nodes.size(); i++) {
		BaseAST* _cur_node = nodes[i];

		if (_cur_node->type == number_literal_ast || _cur_node->type == identifier_ast || _cur_node->type == function_call_ast
			|| _cur_node->type == new_ast || _cur_node->type == character_literal_ast || _cur_node->type == bool_literal_ast || _cur_node->type == keyboard_ast
			|| _cur_node->type == string_literal_ast || _cur_node->type == vector_declaration_ast || _cur_node->type == array_refer_ast || _cur_node->type == cast_ast) {
			result.push_back(_cur_node);
			continue;
		}

		if (stk.empty() || (_cur_node->type == paren_ast && (dynamic_cast<ParenAST*>(_cur_node))->paren == L"(")) {
			stk.push(_cur_node);
			continue;
		}

		if (_cur_node->type == paren_ast && (dynamic_cast<ParenAST*>(_cur_node))->paren == L")") {
			while (!(stk.top()->type == paren_ast && dynamic_cast<ParenAST*>(stk.top())->paren == L"(")) {
				result.push_back(stk.top());
				stk.pop();
			}
			stk.pop(); // remove ')'
			continue;
		}

		if (get_nodes_priority(stk.top()) < get_nodes_priority(_cur_node)) {
			stk.push(_cur_node);
		}
		else {
			while (!stk.empty() && get_nodes_priority(stk.top()) >= get_nodes_priority(_cur_node)) {
				result.push_back(stk.top());
				stk.pop();
			}
			stk.push(_cur_node);
		}
	}

	while (!stk.empty()) {
		result.push_back(stk.top());
		stk.pop();
	}
	return result;
}

BaseAST* Parser::create_expression_ast(std::vector<Token*>& tokens) {

	Token* tok = check_token(tokens);
	std::vector<Token*> expr_toks;

	while (!(tok->get_type() == tok_semi_colon || tok->get_type() == tok_comma)) {
		expr_toks.push_back(lookahead(tokens, tok_none));
		tok = check_token(tokens);
	}

	std::vector<BaseAST*> nodes;

	while (!expr_toks.empty()) {
		nodes.push_back(this->parse_tokens(expr_toks));
	}

	std::vector<BaseAST*> modified_nodes;

	// for neg (-) sign
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->type == operator_ast) {
			if (dynamic_cast<OperatorAST*>(nodes[i])->oper == L"-") {
				if (nodes[i + 1]->type == paren_ast) {
					modified_nodes.push_back(new NumberLiteralAST(L"-1"));
					modified_nodes.push_back(new OperatorAST(L"*"));
					continue;
				}
				else if (nodes[i + 1]->type == identifier_ast) {
					if (i != 0 &&
						!(nodes[i - 1]->type == operator_ast || nodes[i - 1]->type == paren_ast)) {
						modified_nodes.push_back(new OperatorAST(L"+"));
					}
					modified_nodes.push_back(new ParenAST(L"("));
					modified_nodes.push_back(new NumberLiteralAST(L"-1"));
					modified_nodes.push_back(new OperatorAST(L"*"));
					modified_nodes.push_back(nodes[i + 1]);
					modified_nodes.push_back(new ParenAST(L")"));
					i++;
					continue;
				}
			}
		}

		modified_nodes.push_back(nodes[i]);
	}

	/*
	for (int i = 0; i < modified_nodes.size(); i++) {
		if (modified_nodes[i]->type == constant_number_ast) {
			std::wcout << ((NumberAST*)modified_nodes[i])->number_string << " ";
		}
		else if (modified_nodes[i]->type == identifier_ast) {
			std::wcout << ((IdentifierAST*)modified_nodes[i])->identifier << " ";
		}
		else if (modified_nodes[i]->type == operator_ast) {
			std::wcout << ((OperatorAST*)modified_nodes[i])->oper << " ";
		}
		else if (modified_nodes[i]->type == paren_ast) {
			std::wcout << ((ParenAST*)modified_nodes[i])->paren << " ";
		}
	}
	std::cout << std::endl;
	*/

	std::vector<BaseAST*> postfix_result = to_postfix(modified_nodes);
	std::stack<BaseAST*> ast_stack;

	BaseAST* result = nullptr;

	/*
	for (int i = 0; i < postfix_result.size(); i++) {
		if (postfix_result[i]->type == constant_number_ast) {
			std::wcout << ((ConstantNumberAST*)postfix_result[i])->number_string << " ";
		}
		else if (postfix_result[i]->type == identifier_ast) {
			std::wcout << ((IdentifierAST*)postfix_result[i])->identifier << " ";
		}
		else if (postfix_result[i]->type == operator_ast) {
			std::wcout << ((OperatorAST*)postfix_result[i])->oper << " ";
		}
		else if (postfix_result[i]->type == paren_ast) {
			std::wcout << ((ParenAST*)postfix_result[i])->paren << " ";
		}
	}
	*/

	for (int i = 0; i < postfix_result.size(); i++) {
		if (postfix_result[i]->type == operator_ast) {
			BaseAST* lhs = nullptr, * rhs = nullptr;

			if (!ast_stack.empty())
				lhs = ast_stack.top(); ast_stack.pop();
			if (!ast_stack.empty())
				rhs = ast_stack.top(); ast_stack.pop();

			if (lhs == nullptr || rhs == nullptr) {
				// Evoke Error,
			}
			ast_stack.push(new BinExprAST((dynamic_cast<OperatorAST*>(postfix_result[i])->oper), lhs, rhs));
		}
		else {
			ast_stack.push(postfix_result[i]);
		}
	}

	if (!ast_stack.empty())
		result = ast_stack.top();
	else {
		// Evoke Error,
	}

	return result;
}

BaseAST* Parser::parse_tokens(std::vector<Token*>& tokens) {

	Token* first_token = lookahead(tokens, tok_none);
	token_type first_token_type = first_token->get_type();
	BaseAST* result = nullptr;

	switch (first_token_type)
	{

	case tok_constant_number: {
		result = new NumberLiteralAST(first_token->get_identifier());
		break;
	}

	case tok_pls:
	case tok_min:
	case tok_mul:
	case tok_div:
	case tok_pow:
	case tok_mod: {
		result = new OperatorAST(first_token->get_identifier());
		break;
	}

	case tok_var: {
		bool is_static = false;
		std::vector<std::wstring> var_types;
		std::vector<std::wstring> names;
		std::vector<BaseAST*> declarations;
		size_t var_count = 0;

		bool syntax_ended = false;

		while (!syntax_ended) {
			Token* identifier = lookahead(tokens, tok_identifier);

			lookahead(tokens, tok_colon);

			Token* type = lookahead(tokens, tok_none);

			// for delcaration
			BaseAST* declaration = nullptr;
			bool declaration_ended = false;

			while (!declaration_ended) {
				switch (lookahead(tokens, tok_none)->get_type()) {
				case tok_semi_colon: {
					declaration_ended = true;
					syntax_ended = true;
					break;
				}

				case tok_assign: {
					declaration = create_expression_ast(tokens);
					break;
				}

				case tok_comma: {
					declaration_ended = true;
					break;
				}
				}
			}

			var_count++;
			var_types.push_back(type->get_identifier());
			names.push_back(identifier->get_identifier());
			declarations.push_back(declaration);
		}

		result = new VariableDeclarationAST(var_types, names, declarations, var_count);
		break;
	}

	default: {
		//evoke error
		break;
	}
	}

	return result;
}
