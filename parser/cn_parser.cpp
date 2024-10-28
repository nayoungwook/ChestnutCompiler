#include "cn_parser.h"

extern std::unordered_map<std::wstring, ClassAST*> parsed_class_data;
extern std::unordered_map<std::wstring, FunctionDeclarationAST*> parsed_function_data;
extern std::unordered_map<std::wstring, unsigned int> class_id;
extern std::unordered_map<std::wstring, std::unordered_map<std::wstring, MemberFunctionData>> member_function_data;
extern std::unordered_map<std::wstring, std::unordered_map<std::wstring, MemberVariableData>> member_variable_data;

std::vector<Token*> get_parameter_tokens(std::vector<Token*>& tokens) {

	std::vector<Token*> parameter_tokens;

	int paren_count = 0;

	if (tokens[0]->type != tok_l_paren)
		paren_count = 1;

	while (true) {
		Token* _token = pull_token_and_expect(tokens, -1);

		if (_token->type == tok_l_paren) {
			paren_count++;
			if (paren_count == 1) {
				continue;
			}
		}
		if (_token->type == tok_r_paren) {
			paren_count--;

			if (paren_count == 0) {
				break;
			}
		}

		parameter_tokens.push_back(_token);
	}

	return parameter_tokens;
}

static bool is_assign(const Token* token) {
	return token->type == tok_assign ||
		token->type == tok_bin_incre ||
		token->type == tok_bin_decre ||
		token->type == tok_bin_div ||
		token->type == tok_bin_mul;
}

static bool is_operator(const Token* token) {
	return token->type == tok_pls || token->type == tok_min
		|| token->type == tok_mul || token->type == tok_div
		|| token->type == tok_pow || token->type == tok_mod;
}

static bool is_comp(const Token* token) {
	return token->type == tok_equal || token->type == tok_not_equal
		|| token->type == tok_lesser || token->type == tok_greater
		|| token->type == tok_eq_lesser || token->type == tok_eq_greater;
}

static bool is_logic(const Token* token) {
	return token->type == tok_or || token->type == tok_and || token->type == tok_not;
}

int get_nodes_priority(const BaseAST* node) {
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

std::vector<BaseAST*> to_postfix(std::vector<BaseAST*>& nodes) {
	std::vector<BaseAST*> result;
	std::stack<BaseAST*> stk;

	for (int i = 0; i < nodes.size(); i++) {
		BaseAST* _cur_node = nodes[i];

		if (_cur_node->type == number_ast || _cur_node->type == identifier_ast || _cur_node->type == function_call_ast
			|| _cur_node->type == new_ast || _cur_node->type == character_ast || _cur_node->type == bool_ast || _cur_node->type == keyboard_ast
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

BaseAST* create_expr_ast(std::vector<BaseAST*>& nodes) {

	std::vector<BaseAST*> modified_nodes;

	// for neg (-) sign
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i]->type == operator_ast) {
			if (dynamic_cast<OperatorAST*>(nodes[i])->oper == L"-") {
				if (nodes[i + 1]->type == paren_ast) {
					modified_nodes.push_back(new NumberAST(L"-1"));
					modified_nodes.push_back(new OperatorAST(L"*"));
				}
				else {
					if (i != 0 && !(nodes[i - 1]->type == operator_ast || nodes[i - 1]->type == paren_ast)) {
						modified_nodes.push_back(new OperatorAST(L"+"));
					}
					modified_nodes.push_back(new ParenAST(L"("));
					modified_nodes.push_back(new NumberAST(L"-1"));
					modified_nodes.push_back(new OperatorAST(L"*"));
					modified_nodes.push_back(nodes[i + 1]);
					modified_nodes.push_back(new ParenAST(L")"));
					i++;
				}
				continue;
			}
		}

		modified_nodes.push_back(nodes[i]);
	}

	/*
	for (int i = 0; i < modified_nodes.size(); i++) {
		if (modified_nodes[i]->type == number_ast) {
			std::cout << ((NumberAST*)modified_nodes[i])->number_string << " ";
		}
		else if (modified_nodes[i]->type == identifier_ast) {
			std::cout << ((IdentifierAST*)modified_nodes[i])->identifier << " ";
		}
		else if (modified_nodes[i]->type == operator_ast) {
			std::cout << ((OperatorAST*)modified_nodes[i])->oper << " ";
		}
		else if (modified_nodes[i]->type == paren_ast) {
			std::cout << ((ParenAST*)modified_nodes[i])->paren << " ";
		}
	}
	std::cout << std::endl;
	*/

	std::vector<BaseAST*> postfix_result = to_postfix(modified_nodes);
	std::stack<BaseAST*> ast_stack;

	BaseAST* result = nullptr;
	/*
	for (int i = 0; i < postfix_result.size(); i++) {
		if (postfix_result[i]->type == number_ast) {
			std::wcout << ((NumberAST*)postfix_result[i])->number_string << " ";
		}
		else if (postfix_result[i]->type == identifier_ast) {
			std::wcout << ((IdentifierAST*)postfix_result[i])->identifier << " ";
		}
		else if (postfix_result[i]->type == string_literal_ast) {
			std::wcout << ((StringLiteralAST*)postfix_result[i])->str_literal << " ";
		}
		else if (postfix_result[i]->type == operator_ast) {
			std::wcout << ((OperatorAST*)postfix_result[i])->oper << " ";
		}
		else if (postfix_result[i]->type == keyboard_ast) {
			std::wcout << ((KeyboardAST*)postfix_result[i])->pressed_key << " ";
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
				CHESTNUT_THROW_ERROR(L"Failed to create expression due to unknown error", "FAILED_TO_CREATE_EXPRESSION", "0x15", 0);
			}
			ast_stack.push(new BinExprAST((dynamic_cast<OperatorAST*>(postfix_result[i])->oper), lhs, rhs));
		}
		else {
			ast_stack.push(postfix_result[i]);
		}
	}

	if (!ast_stack.empty())
		result = ast_stack.top();
	else
		CHESTNUT_THROW_ERROR(L"Failed to create expression due to unknown error", "FAILED_TO_CREATE_EXPRESSION", "0x15", 0);

	return result;
}

std::vector<std::vector<Token*>> get_function_parameter_tokens(std::vector<Token*>& tokens) {
	std::vector<Token*> function_parameter_tokens = get_parameter_tokens(tokens);

	std::vector<std::vector<Token*>> result;
	std::vector<Token*> _tokens;

	int paren_count = 0;

	for (int i = 0; i < function_parameter_tokens.size(); i++) {
		if (function_parameter_tokens[i]->type == tok_l_paren)
			paren_count++;
		if (function_parameter_tokens[i]->type == tok_r_paren)
			paren_count--;

		if (function_parameter_tokens[i]->type == tok_comma && paren_count == 0) {
			result.push_back(_tokens);

			if (_tokens.size() == 0) {
				CHESTNUT_THROW_ERROR(L"Invalid parameter. Please check your parameter", "INVALID_PARAMETER", "012", function_parameter_tokens[0]->line);
			}

			_tokens.clear();
			continue;
		}
		_tokens.push_back(function_parameter_tokens[i]);
	}

	if (!_tokens.empty()) {
		result.push_back(_tokens);
		_tokens.clear();
	}

	return result;
}

std::vector<Token*> get_block_tokens(std::vector<Token*>& tokens) {
	int bracket_count = 0;
	std::vector<Token*> body_tokens;


	while (true) {
		Token* _token = pull_token_and_expect(tokens, -1);

		if (_token->type == tok_l_bracket) {
			bracket_count++;
			if (bracket_count == 1) {
				continue;
			}
		}
		if (_token->type == tok_r_bracket) {
			bracket_count--;

			if (bracket_count == 0) {
				break;
			}
		}

		body_tokens.push_back(_token);
	}

	return body_tokens;
}

Token* check_token(std::vector<Token*>& tokens) {
	if (tokens.size() == 0) {
		return new Token(tok_null, L"", 0);
	}
	Token* pull = tokens.front();

	return pull;
}

Token* pull_token_and_expect(std::vector<Token*>& tokens, int token_type) { // if token type is -1 , it can be anything.
	if (tokens.size() == 0) {
		CHESTNUT_THROW_ERROR(L"there\'s no token.", "NO_TOKEN", "010", -1);
	}
	Token* pull = tokens.front();
	tokens.erase(tokens.begin());

	if (token_type != -1 && pull->type != token_type) {
		std::wstring w_expected_token;
		w_expected_token.assign(token_string[token_type].begin(), token_string[token_type].end());

		std::wstring w_pulled_type;
		w_pulled_type.assign(token_string[pull->type].begin(), token_string[pull->type].end());

		std::wstring w_pulled_token;
		w_pulled_token.assign(pull->identifier.begin(), pull->identifier.end());

		CHESTNUT_THROW_ERROR(L"Unexpected syntax error, expected : " + w_expected_token + L" but pulled : " + w_pulled_token + L" is type " + w_pulled_type
			, "UNEXPECTED_TOKEN", "009", pull->line);
	}

	return pull;
}

BaseAST* get_null_delcaration_ast(const std::wstring& type) {
	if (type == L"number")
		return new NumberAST(L"0");
	else if (type == L"bool")
		return new BoolAST(true);
	else if (type == L"array") {
		std::vector<BaseAST*> content;

		return new ArrayDeclarationAST(content);
	}
	else
		return nullptr;
}

VariableDeclarationAST* create_variable_declaration_ast(std::vector<Token*>& tokens) {
	Token* tok = nullptr;
	std::vector<std::wstring> names;
	std::vector<std::wstring> types;
	std::vector<std::wstring> array_var_types;
	std::vector<size_t> array_sizes;
	std::vector<BaseAST*> declarations;

	while (true) {
		tok = pull_token_and_expect(tokens, tok_identifier);
		names.push_back(tok->identifier);

		std::wstring array_var_type = L"";

		tok = pull_token_and_expect(tokens, tok_colon);

		tok = pull_token_and_expect(tokens, -1);

		size_t size = 0;

		array_sizes.push_back(size);

		std::wstring type = tok->identifier;
		types.push_back(type);

		if (tok->type == tok_array) {
			pull_token_and_expect(tokens, tok_greater);

			tok = pull_token_and_expect(tokens, -1);

			array_var_type = tok->identifier;

			pull_token_and_expect(tokens, tok_lesser);
		}

		array_var_types.push_back(array_var_type);

		tok = pull_token_and_expect(tokens, -1);

		if (tok->type == tok_assign) {
			std::vector<Token*> declaration_tokens;
			Token* _mono_declaration_token = check_token(tokens);

			// check until meet ; or , and get tokens
			int paren_count = 0, bracket_count = 0;
			while (_mono_declaration_token->type != tok_semi_colon && !(_mono_declaration_token->type == tok_comma && paren_count == 0 && bracket_count == 0)) {
				if (_mono_declaration_token->type == tok_l_paren)
					paren_count++;
				if (_mono_declaration_token->type == tok_r_paren)
					paren_count--;
				if (_mono_declaration_token->type == tok_l_bracket)
					bracket_count++;
				if (_mono_declaration_token->type == tok_r_bracket)
					bracket_count--;

				declaration_tokens.push_back(_mono_declaration_token);
				pull_token_and_expect(tokens, -1);
				_mono_declaration_token = check_token(tokens);
			}

			// make it ast
			std::vector<BaseAST*> declaration_asts;

			while (!declaration_tokens.empty()) {
				declaration_asts.push_back(parse(declaration_tokens));
			}

			// push
			declarations.push_back(create_expr_ast(declaration_asts));

			// process left ; or ,
			if (check_token(tokens)->type == tok_comma) {
				pull_token_and_expect(tokens, tok_comma);
			}
			if (check_token(tokens)->type == tok_semi_colon) {
				pull_token_and_expect(tokens, tok_semi_colon);
				break;
			}
		}
		else if (tok->type == tok_comma) {
			declarations.push_back(nullptr);
		}
		else if (tok->type == tok_semi_colon) {
			declarations.push_back(get_null_delcaration_ast(types[types.size() - 1]));

			if (tok->type == tok_semi_colon) {
				break;
			}
		}
	}

	return new VariableDeclarationAST(types, names, declarations, array_var_types, names.size());
}

BinExprAST* create_assign_ast(std::vector<Token*>& tokens, IdentifierAST* target) {
	std::wstring oper = pull_token_and_expect(tokens, -1)->identifier;
	std::vector<Token*> rhs_tokens;

	Token* tok = pull_token_and_expect(tokens, -1);
	while (tok->type != tok_semi_colon) {
		rhs_tokens.push_back(tok);
		tok = pull_token_and_expect(tokens, -1);
		if (tok->type == tok_semi_colon) break;
	}

	std::vector<BaseAST*> _rhs_asts;

	while (!rhs_tokens.empty()) {
		BaseAST* _rhs_ast = parse(rhs_tokens);
		_rhs_asts.push_back(_rhs_ast);
	}

	BinExprAST* expression = static_cast<BinExprAST*>(create_expr_ast(_rhs_asts));

	return new BinExprAST(oper, target, expression);
}

FunctionCallAST* create_function_call_ast(std::vector<Token*>& tokens, IdentifierAST* target) {
	std::wstring name = target->identifier;

	std::vector<std::vector<Token*>> parameter_tokens = get_function_parameter_tokens(tokens);
	std::vector<std::vector<BaseAST*>> parameter_asts;

	for (int i = 0; i < parameter_tokens.size(); i++) {
		parameter_asts.push_back({});
		while (!parameter_tokens[i].empty()) {
			parameter_asts[parameter_asts.size() - 1].push_back(parse(parameter_tokens[i]));
		}
	}

	std::vector<BaseAST*> parameters;

	for (int i = 0; i < parameter_asts.size(); i++) {
		BaseAST* _parameter_ast = create_expr_ast(parameter_asts[i]);
		parameters.push_back(_parameter_ast);
	}

	Token* _check_token = check_token(tokens);

	if (_check_token != nullptr && _check_token->type == tok_semi_colon)
		pull_token_and_expect(tokens, tok_semi_colon);

	return new FunctionCallAST(name, parameters);
}

BaseAST* create_increase_identifier_ast(std::vector<Token*>& tokens, IdentifierAST* target) {
	pull_token_and_expect(tokens, tok_incre);
	BinExprAST* result = new BinExprAST(L"++", target, nullptr);

	if (tokens.size() != 0) {
		if (check_token(tokens)->type == tok_semi_colon)
			pull_token_and_expect(tokens, tok_semi_colon);
	}

	return result;
}

BaseAST* create_decrease_identifier_ast(std::vector<Token*>& tokens, IdentifierAST* target) {

	pull_token_and_expect(tokens, tok_decre);
	BinExprAST* result = new BinExprAST(L"--", target, nullptr);

	if (tokens.size() != 0) {
		if (check_token(tokens)->type == tok_semi_colon)
			pull_token_and_expect(tokens, tok_semi_colon);
	}

	return result;
}

ArrayReferAST* create_array_refer_ast(std::vector<Token*>& tokens, IdentifierAST* target) {
	ArrayReferAST* result = nullptr;
	std::vector<BaseAST*> indexes;

	while (check_token(tokens)->type == tok_l_sq_bracket) {
		pull_token_and_expect(tokens, tok_l_sq_bracket);

		std::vector<Token*> index_tokens;

		while (check_token(tokens)->type != tok_r_sq_bracket) {
			index_tokens.push_back(pull_token_and_expect(tokens, -1));
		}

		pull_token_and_expect(tokens, tok_r_sq_bracket);

		std::vector<BaseAST*> declaration_asts;

		while (!index_tokens.empty()) {
			declaration_asts.push_back(parse(index_tokens));
		}

		BaseAST* _index = create_expr_ast(declaration_asts);

		indexes.push_back(_index);
	}

	result = new ArrayReferAST(target->identifier, indexes);

	return result;
}

KeyboardAST* create_kb_ast(std::vector<Token*>& tokens, IdentifierAST* target) {
	KeyboardAST* result = new KeyboardAST(pull_token_and_expect(tokens, -1)->identifier);
	return result;
}

BaseAST* create_identifier_ast(std::vector<Token*>& tokens, IdentifierAST* target) {
	bool _is_assign = tokens.size() != 0 && is_assign(check_token(tokens));
	bool _is_function_call = tokens.size() != 0 && check_token(tokens)->type == tok_l_paren;
	bool _is_comp = tokens.size() != 0 && is_comp(check_token(tokens));
	bool _is_incre = tokens.size() != 0 && check_token(tokens)->type == tok_incre;
	bool _is_decre = tokens.size() != 0 && check_token(tokens)->type == tok_decre;
	bool _is_array_reference = tokens.size() != 0 && check_token(tokens)->type == tok_l_sq_bracket;
	bool _is_kb = target->identifier == L"key";

	if (_is_assign)
		return create_assign_ast(tokens, target);
	else if (_is_function_call)
		return create_function_call_ast(tokens, target);
	else if (_is_incre)
		return create_increase_identifier_ast(tokens, target);
	else if (_is_decre)
		return create_decrease_identifier_ast(tokens, target);
	else if (_is_kb)
		return create_kb_ast(tokens, target);
	else if (_is_array_reference) {
		ArrayReferAST* refer_ast = create_array_refer_ast(tokens, target);

		BaseAST* result = create_identifier_ast(tokens, refer_ast);
		return result;
	}
	else
		return target;
}

FunctionDeclarationAST* create_function_declaration_ast(std::vector<Token*>& tokens) {
	FunctionDeclarationAST* ast = nullptr;
	std::wstring name = pull_token_and_expect(tokens, tok_identifier)->identifier;

	std::vector<std::vector<Token*>> parameter_tokens = get_function_parameter_tokens(tokens);
	std::vector<VariableDeclarationAST*> parameters;

	for (int i = 0; i < parameter_tokens.size(); i++) {
		if (parameter_tokens[i].size() >= 3) {

			std::wstring _type = parameter_tokens[i][2]->identifier;
			std::wstring _name = parameter_tokens[i][0]->identifier;

			std::vector<std::wstring> types = { _type };
			std::vector<std::wstring> _array_types;
			std::vector<std::wstring> name = { _name };

			if (_type == L"array") {
				if (parameter_tokens[i].size() > 2)
					_array_types.push_back(parameter_tokens[i][4]->identifier);
				else {
					std::cout << "Error! array should have a type" << std::endl;
					exit(EXIT_FAILURE);
				}
			}

			std::vector<BaseAST*> _decl;
			_decl.push_back(nullptr);

			parameters.push_back(new VariableDeclarationAST(types, name, _decl, _array_types, 1));
		}
		else {
			CHESTNUT_THROW_ERROR(L"Wrong parameter syntax.", "WRONG_PARAMETER_SYNTAX", "0x23", tokens[0]->line);
		}
	}

	pull_token_and_expect(tokens, tok_colon);
	std::wstring return_type = pull_token_and_expect(tokens, -1)->identifier;

	std::wstring array_var_type = L"";

	if (return_type == L"array") {
		pull_token_and_expect(tokens, tok_greater);

		array_var_type = pull_token_and_expect(tokens, -1)->identifier;

		pull_token_and_expect(tokens, tok_lesser);
	}

	std::vector<Token*> body_tokens = get_block_tokens(tokens);
	std::vector<BaseAST*> body;

	while (!body_tokens.empty()) {
		body.push_back(parse(body_tokens));
	}

	ast = new FunctionDeclarationAST(name, parameters, return_type, array_var_type);
	ast->body = body;

	parsed_function_data.insert(std::make_pair(name, ast));

	return ast;
}

void assign_member_variable_data(ClassAST* class_ast, std::unordered_map<std::wstring, MemberVariableData>& member_variables) {
	for (int i = 0; i < class_ast->variables.size(); i++) {
		VariableDeclarationAST* ast = (VariableDeclarationAST*)class_ast->variables[i];

		for (int j = 0; j < ast->var_count; j++) {
			MemberVariableData data;

			std::wstring name = ast->names[j];

			data.name = name;
			data.access_modifier = ast->access_modifier;
			data.id = member_variables.size();

			if (ast->var_types[j] != L"array") {
				data.type = ast->var_types[j];
				data.is_array = false;
			}
			else {
				data.type = ast->array_var_types[j];
				data.is_array = true;
			}

			member_variables.insert(std::make_pair(name, data));
		}
	}
}

void appply_member_data(ClassAST* class_ast, std::vector<BaseAST*>& function_asts) {
	std::unordered_map<std::wstring, MemberFunctionData> member_functions;
	for (int i = 0; i < function_asts.size(); i++) {
		FunctionDeclarationAST* ast = (FunctionDeclarationAST*)function_asts[i];

		MemberFunctionData data;
		std::wstring name = ast->function_name;

		data.return_type = ast->return_type;
		data.access_modifier = ast->access_modifier;
		data.name = name;
		data.id = i;

		member_functions.insert(std::make_pair(name, data));
	}

	member_function_data.insert(std::make_pair(class_ast->name, member_functions));

	std::unordered_map<std::wstring, MemberVariableData> member_variables;
	assign_member_variable_data(class_ast, member_variables);
	member_variable_data.insert(std::make_pair(class_ast->name, member_variables));
}

ClassAST* create_class_declaration_ast(std::vector<Token*>& tokens) {
	std::wstring name = pull_token_and_expect(tokens, tok_identifier)->identifier;

	std::wstring parent_type = L"";
	if (check_token(tokens)->type == tok_extends) {
		pull_token_and_expect(tokens, tok_extends);
		parent_type = pull_token_and_expect(tokens, tok_identifier)->identifier;
	}

	std::vector<Token*> body_tokens = get_block_tokens(tokens);

	std::vector<BaseAST*> function_asts;
	std::vector<BaseAST*> variable_asts;

	ClassAST* ast = new ClassAST(name, parent_type);

	while (!body_tokens.empty()) {
		BaseAST* _ast = parse(body_tokens);

		if (_ast->type == variable_declaration_ast)
			variable_asts.push_back(_ast);
		if (_ast->type == function_declaration_ast)
			function_asts.push_back(_ast);
		if (_ast->type == constructor_declaration_ast)
			ast->constructor.push_back((ConstructorDeclarationAST*)_ast);
	}

	if (ast->constructor.size() == 0) {
		std::vector< VariableDeclarationAST*> parameters;
		ast->constructor.push_back(new ConstructorDeclarationAST(parameters));
	}

	ast->functions = function_asts;
	ast->variables = variable_asts;

	parsed_class_data.insert(std::make_pair(name, ast));
	class_id.insert(std::make_pair(name, class_id.size()));
	appply_member_data(ast, function_asts);

	std::vector<Token*>().swap(body_tokens);

	return ast;
}

ObjectAST* create_object_declaration_ast(std::vector<Token*>& tokens) {

	std::wstring name = pull_token_and_expect(tokens, tok_identifier)->identifier;

	std::wstring parent_type = L"";
	if (check_token(tokens)->type == tok_extends) {
		pull_token_and_expect(tokens, tok_extends);
		parent_type = pull_token_and_expect(tokens, tok_identifier)->identifier;
	}

	std::vector<Token*> body_tokens = get_block_tokens(tokens);

	std::vector<BaseAST*> function_asts;
	std::vector<BaseAST*> variable_asts;
	std::set<std::wstring> declarated;

	ObjectAST* ast = new ObjectAST(name, parent_type);

	std::vector<std::wstring> _type = { L"vector", L"number", L"number", L"number", L"texture" };
	std::vector<std::wstring> _name = { L"position", L"width", L"height", L"rotation", L"sprite" };
	std::vector<std::wstring> _var_types = { L"", L"", L"", L"", L"" };
	std::vector<BaseAST*> vector_decl = { new NumberAST(L"0"), new NumberAST(L"0") };

	std::vector<BaseAST*> _decl = { new VectorDeclarationAST(vector_decl), new NumberAST(L"100") , new NumberAST(L"100"), new NumberAST(L"0"), new IdentifierAST(L"null") };

	VariableDeclarationAST* _builtin_decl = new VariableDeclarationAST(_type, _name, _decl, _var_types, _decl.size());
	_builtin_decl->access_modifier = L"public";

	variable_asts.push_back(_builtin_decl);

	std::map<std::wstring, bool> functions_satisfied =
	{
		std::make_pair(L"init", false),
		std::make_pair(L"tick", false),
		std::make_pair(L"render", false),
	};

	while (!body_tokens.empty()) {
		BaseAST* _ast = parse(body_tokens);

		if (_ast->type == variable_declaration_ast)
			variable_asts.push_back(_ast);
		if (_ast->type == function_declaration_ast) {
			function_asts.push_back(_ast);

			if (((FunctionDeclarationAST*)_ast)->access_modifier == L"public") {
				if (!((FunctionDeclarationAST*)_ast)->is_static) {
					if (
						dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name == L"init" ||
						dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name == L"tick" ||
						dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name == L"render"
						) {
						functions_satisfied[dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name] = true;
					}
				}
			}
		}

		if (_ast->type == constructor_declaration_ast)
			ast->constructor.push_back((ConstructorDeclarationAST*)_ast);

		declarated.insert(name);
	}

	if (ast->constructor.size() == 0) {
		std::vector< VariableDeclarationAST*> parameters;
		ast->constructor.push_back(new ConstructorDeclarationAST(parameters));
	}

	for (std::pair<std::wstring, bool> satisfied : functions_satisfied) {
		if (!satisfied.second) {
			std::vector<VariableDeclarationAST*> parameters;
			FunctionDeclarationAST* function_declaration_ast = new FunctionDeclarationAST(satisfied.first, parameters, L"void", L"");
			function_declaration_ast->access_modifier = L"public";
			function_asts.push_back(function_declaration_ast);
		}
	}

	ast->functions = function_asts;
	ast->variables = variable_asts;

	parsed_class_data.insert(std::make_pair(name, ast));
	class_id.insert(std::make_pair(name, class_id.size()));
	appply_member_data(ast, function_asts);

	std::vector<Token*>().swap(body_tokens);

	return ast;
}

SceneAST* create_scene_declaration_ast(std::vector<Token*>& tokens) {

	std::wstring name = pull_token_and_expect(tokens, -1)->identifier;

	std::wstring parent_type = L"";
	if (check_token(tokens)->type == tok_extends) {
		pull_token_and_expect(tokens, tok_extends);
		parent_type = pull_token_and_expect(tokens, tok_identifier)->identifier;
	}

	std::vector<Token*> body_tokens = get_block_tokens(tokens);

	std::vector<BaseAST*> function_asts;
	std::vector<BaseAST*> variable_asts;

	SceneAST* ast = new SceneAST(name, parent_type);
	std::map<std::wstring, bool> functions_satisfied =
	{
		std::make_pair(L"init", false),
		std::make_pair(L"tick", false),
		std::make_pair(L"render", false),
	};

	while (!body_tokens.empty()) {
		BaseAST* _ast = parse(body_tokens);

		if (_ast->type == variable_declaration_ast)
			variable_asts.push_back(_ast);
		if (_ast->type == function_declaration_ast) {
			function_asts.push_back(_ast);

			if (((FunctionDeclarationAST*)_ast)->access_modifier == L"public") {
				if (!((FunctionDeclarationAST*)_ast)->is_static) {
					if (
						dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name == L"init" ||
						dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name == L"tick" ||
						dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name == L"render"
						) {
						functions_satisfied[dynamic_cast<FunctionDeclarationAST*>(_ast)->function_name] = true;
					}
				}
			}
		}
		if (_ast->type == constructor_declaration_ast)
			ast->constructor.push_back((ConstructorDeclarationAST*)_ast);
	}

	if (ast->constructor.size() == 0) {
		std::vector<VariableDeclarationAST*> parameters;
		ast->constructor.push_back(new ConstructorDeclarationAST(parameters));
	}

	for (std::pair<std::wstring, bool> satisfied : functions_satisfied) {
		if (!satisfied.second) {
			std::vector<VariableDeclarationAST*> parameters;
			FunctionDeclarationAST* function_declaration_ast = new FunctionDeclarationAST(satisfied.first, parameters, L"void", L"");
			function_declaration_ast->access_modifier = L"public";
			function_asts.push_back(function_declaration_ast);
		}
	}

	ast->functions = function_asts;
	ast->variables = variable_asts;

	std::vector<Token*>().swap(body_tokens);

	parsed_class_data.insert(std::make_pair(name, ast));
	class_id.insert(std::make_pair(name, class_id.size()));
	appply_member_data(ast, function_asts);

	return ast;
}

ReturnAST* create_return_ast(std::vector<Token*>& tokens) {
	std::vector<Token*> declaration_tokens;
	Token* current_token = check_token(tokens);

	while (current_token->type != tok_semi_colon) {
		declaration_tokens.push_back(current_token);
		pull_token_and_expect(tokens, -1);
		current_token = check_token(tokens);
	}

	pull_token_and_expect(tokens, tok_semi_colon);

	std::vector<BaseAST*> declaration_asts;

	while (!declaration_tokens.empty()) {
		declaration_asts.push_back(parse(declaration_tokens));
	}

	BaseAST* declaration_ast = create_expr_ast(declaration_asts);

	return new ReturnAST(declaration_ast);
}

NewAST* create_new_ast(std::vector<Token*>& tokens) {
	NewAST* result = nullptr;

	std::wstring class_name = pull_token_and_expect(tokens, tok_identifier)->identifier;

	std::vector<std::vector<Token*>> parameter_tokens = get_function_parameter_tokens(tokens);
	std::vector<std::vector<BaseAST*>> parameter_asts;

	for (int i = 0; i < parameter_tokens.size(); i++) {
		parameter_asts.push_back({});
		while (!parameter_tokens[i].empty()) {
			parameter_asts[parameter_asts.size() - 1].push_back(parse(parameter_tokens[i]));
		}
	}

	std::vector<BaseAST*> parameters;

	for (int i = 0; i < parameter_asts.size(); i++) {
		BaseAST* _parameter_ast = create_expr_ast(parameter_asts[i]);
		parameters.push_back(_parameter_ast);
	}

	result = new NewAST(class_name);
	result->parameters = parameters;

	return result;
}

ImportAST* create_import_ast(std::vector<Token*>& tokens) {
	ImportAST* result = nullptr;

	std::wstring import_name = pull_token_and_expect(tokens, tok_string_identifier)->identifier;
	import_name = import_name.substr(1, import_name.size() - 2);

	compile_imported_file(import_name);

	result = new ImportAST(import_name);

	return result;
}

BaseAST* get_if_statement_condition(std::vector<Token*>& tokens) {
	std::vector<Token*> parameter_tokens = get_parameter_tokens(tokens);
	std::vector<BaseAST*> parameter_asts;

	while (!parameter_tokens.empty()) {
		parameter_asts.push_back(parse(parameter_tokens));
	}

	return create_expr_ast(parameter_asts);
}

IfStatementAST* create_if_statement_ast(std::vector<Token*>& tokens, if_statement_type statement_type, IfStatementAST* parent_statement_ast) {
	IfStatementAST* result = nullptr;

	BaseAST* condition = nullptr;

	if (statement_type != statement_else)
		condition = get_if_statement_condition(tokens);

	result = new IfStatementAST(condition);

	result->statement_type = statement_type;
	std::vector<Token*> body_tokens;
	if (check_token(tokens)->type == tok_l_bracket) {
		body_tokens = get_block_tokens(tokens);
	}
	else {
		while (check_token(tokens)->type != tok_semi_colon) {
			body_tokens.push_back(pull_token_and_expect(tokens, -1));
		}
		body_tokens.push_back(pull_token_and_expect(tokens, tok_semi_colon));
	}

	std::vector<BaseAST*> body;
	while (!body_tokens.empty()) {
		body.push_back(parse(body_tokens));
	}

	result->body = body;

	if (check_token(tokens)->type == tok_else) {
		pull_token_and_expect(tokens, tok_else);
		if_statement_type statement_type = statement_else;

		if (check_token(tokens)->type == tok_if) {
			statement_type = statement_elif;
			pull_token_and_expect(tokens, tok_if);
		}

		if (parent_statement_ast == nullptr)
			result->additional_statements.push_back(
				create_if_statement_ast(tokens, statement_type, result));
		else
			parent_statement_ast->additional_statements.push_back(
				create_if_statement_ast(tokens, statement_type, parent_statement_ast));
	}

	return result;
}

ForStatementAST* create_for_statement_ast(std::vector<Token*>& tokens) {
	std::vector<Token*> parameter_tokens = get_parameter_tokens(tokens);

	std::vector<std::vector<Token*>> separated_tokens(3);
	int seprate_index = 0;

	for (Token* tok : parameter_tokens) {
		if (tok->type == tok_semi_colon) {
			seprate_index++;
			continue;
		}
		separated_tokens[seprate_index].push_back(tok);
	}

	separated_tokens[0].push_back(new Token(tok_semi_colon, L";", 0));
	separated_tokens[2].push_back(new Token(tok_semi_colon, L";", 0));

	std::vector<BaseAST*> condition_asts;

	while (!separated_tokens[1].empty()) {
		condition_asts.push_back(parse(separated_tokens[1]));
	}

	BaseAST* init = parse(separated_tokens[0]),
		* condition = create_expr_ast(condition_asts),
		* step = parse(separated_tokens[2]);

	ForStatementAST* result = new ForStatementAST(init, condition, step);

	std::vector<Token*> body_tokens = get_block_tokens(tokens);

	std::vector<BaseAST*> body;
	while (!body_tokens.empty()) {
		body.push_back(parse(body_tokens));
	}

	result->body = body;

	return result;
}

WhileStatementAST* create_while_statement_ast(std::vector<Token*>& tokens) {
	WhileStatementAST* result = nullptr;

	std::vector<Token*> parameter_tokens = get_parameter_tokens(tokens);

	std::vector<BaseAST*> condition_asts;

	while (!parameter_tokens.empty()) {
		condition_asts.push_back(parse(parameter_tokens));
	}

	BaseAST* condition = create_expr_ast(condition_asts);

	result = new WhileStatementAST(condition);

	std::vector<Token*> body_tokens = get_block_tokens(tokens);

	std::vector<BaseAST*> body;
	while (!body_tokens.empty()) {
		body.push_back(parse(body_tokens));
	}

	result->body = body;

	return result;
}

ArrayDeclarationAST* create_array_declaration_ast(std::vector<Token*>& tokens) {
	int bracket_count = 1;

	std::vector<Token*> element_tokens;
	std::vector<BaseAST*> elements;

	while (bracket_count != 0) {
		Token* tok = pull_token_and_expect(tokens, -1);

		if (tok->type == tok_l_bracket) {
			bracket_count++;
		}
		if (tok->type == tok_r_bracket) {
			bracket_count--;

			if (bracket_count == 0)
				break;
		}

		if (tok->type == tok_comma && bracket_count == 1) {
			std::vector<BaseAST*> _element_nodes;

			while (!element_tokens.empty()) {
				_element_nodes.push_back(parse(element_tokens));
			}
			elements.push_back(create_expr_ast(_element_nodes));

			continue;
		}

		element_tokens.push_back(tok);
	}


	if (!element_tokens.empty()) {
		std::vector<BaseAST*> _element_nodes;
		while (!element_tokens.empty()) {
			_element_nodes.push_back(parse(element_tokens));
		}
		elements.push_back(create_expr_ast(_element_nodes));
	}

	return new ArrayDeclarationAST(elements);
}

ConstructorDeclarationAST* create_constructor_declaration_ast(std::vector<Token*>& tokens) {
	ConstructorDeclarationAST* result = nullptr;

	std::vector<std::vector<Token*>> parameter_tokens = get_function_parameter_tokens(tokens);
	std::vector<VariableDeclarationAST*> parameters;

	for (int i = 0; i < parameter_tokens.size(); i++) {
		std::wstring _type = parameter_tokens[i][2]->identifier;
		std::wstring _name = parameter_tokens[i][0]->identifier;

		std::vector<std::wstring> types = { _type };
		std::vector<std::wstring> _array_types;
		std::vector<std::wstring> name = { _name };

		if (_type == L"array") {
			if (parameter_tokens[i].size() > 2)
				_array_types.push_back(parameter_tokens[i][4]->identifier);
			else {
				CHESTNUT_THROW_ERROR(L"Error! array must contain type.", "ARRAY_MUST_CONTAIN_TYPE", "0x14", tokens[0]->line);
				std::cout << "Error! array should have a type" << std::endl;
				exit(EXIT_FAILURE);
			}
		}

		std::vector<BaseAST*> _decl;
		_decl.push_back(nullptr);

		parameters.push_back(new VariableDeclarationAST(types, name, _decl, _array_types, 1));
	}

	std::vector<Token*> body_tokens = get_block_tokens(tokens);
	std::vector<BaseAST*> body;

	while (!body_tokens.empty()) {
		body.push_back(parse(body_tokens));
	}

	result = new ConstructorDeclarationAST(parameters);
	result->body = body;

	return result;
}

NotAST* create_not_ast(std::vector<Token*>& tokens) {
	NotAST* result = nullptr;

	std::vector<Token*> expression_tokens;
	BaseAST* expression;

	if (check_token(tokens)->type == tok_l_paren) {

		std::vector<BaseAST*> expression_nodes;

		expression_tokens.push_back(pull_token_and_expect(tokens, tok_l_paren));
		int paren_count = 1;
		while (paren_count != 0) {
			if (check_token(tokens)->type == tok_l_paren) {
				paren_count++;
			}
			if (check_token(tokens)->type == tok_r_paren) {
				paren_count--;
			}
			expression_tokens.push_back(pull_token_and_expect(tokens, -1));
		}

		while (!expression_tokens.empty()) {
			expression_nodes.push_back(parse(expression_tokens));
		}

		expression = create_expr_ast(expression_nodes);
	}
	else {
		expression = parse(tokens);
	}

	result = new NotAST(expression);

	return result;
}

VectorDeclarationAST* create_vector_declaration_ast(std::vector<Token*>& tokens) {
	VectorDeclarationAST* result = nullptr;

	std::vector<std::vector<Token*>> element_tokens = get_function_parameter_tokens(tokens);

	std::vector<std::vector<BaseAST*>> element_asts;

	for (int i = 0; i < element_tokens.size(); i++) {
		element_asts.push_back({});
		while (!element_tokens[i].empty()) {
			element_asts[element_asts.size() - 1].push_back(parse(element_tokens[i]));
		}
	}

	std::vector<BaseAST*> elements;

	if (element_asts.size() > 4) {
		//		new VectorElementsDeclaredMoreThanFourError(check_token(tokens)->line);
	}

	for (int i = 0; i < element_asts.size(); i++) {
		BaseAST* _element_ast = create_expr_ast(element_asts[i]);
		elements.push_back(_element_ast);
	}

	result = new VectorDeclarationAST(elements);

	return result;
}

CastAST* create_cast_ast(std::vector<Token*>& tokens) {
	CastAST* result = nullptr;

	pull_token_and_expect(tokens, tok_l_paren);
	Token* type = pull_token_and_expect(tokens, -1);
	pull_token_and_expect(tokens, tok_r_paren);

	std::vector<Token*> expr_tokens;

	BaseAST* target_ast = nullptr;

	if (check_token(tokens)->type == tok_l_paren) {
		int paren_count = 0;

		do {
			Token* tok = pull_token_and_expect(tokens, -1);

			if (tok->type == tok_l_paren) {
				paren_count++;
				if (paren_count == 1) continue;
			}
			if (tok->type == tok_r_paren) {
				paren_count--;
				if (paren_count == 0) continue;
			}

			expr_tokens.push_back(tok);

		} while (paren_count != 0);

		std::vector<BaseAST*> expr_asts;

		while (!expr_tokens.empty()) {
			expr_asts.push_back(parse(expr_tokens));
		}

		target_ast = create_expr_ast(expr_asts);
	}
	else {
		target_ast = parse(tokens);
	}

	result = new CastAST(type->identifier, target_ast);

	return result;
}

LoadAST* create_load_ast(std::vector<Token*>& tokens) {
	LoadAST* result = nullptr;

	std::wstring name = pull_token_and_expect(tokens, tok_identifier)->identifier;
	std::wstring path = pull_token_and_expect(tokens, tok_string_identifier)->identifier;

	result = new LoadAST(name, path);

	return result;
}

FontAST* create_font_ast(std::vector<Token*>& tokens) {
	FontAST* result = nullptr;

	std::wstring name = pull_token_and_expect(tokens, tok_identifier)->identifier;
	std::wstring path = pull_token_and_expect(tokens, tok_string_identifier)->identifier;

	result = new FontAST(name, path);

	return result;
}

OptionAST* create_option_ast(std::wstring const& option_name, std::vector<Token*>& tokens) {
	OptionAST* result = nullptr;

	result = new OptionAST(option_name.substr(1, option_name.length() - 1));

	return result;
}

BaseAST* parse(std::vector<Token*>& tokens) {
	BaseAST* result = nullptr;
	int token_size = 0;

	if (tokens.size() == 0) return nullptr;

	Token* first_token = pull_token_and_expect(tokens, -1);

	if (first_token->type == tok_identifier_number) {
		result = new NumberAST(first_token->identifier);
	}
	else if (first_token->type == tok_string_identifier) {
		result = new StringLiteralAST(first_token->identifier);
	}
	else if (first_token->type == tok_not) {
		result = create_not_ast(tokens);
	}
	else if (is_operator(first_token) || is_comp(first_token) || is_logic(first_token)) {
		result = new OperatorAST(first_token->identifier);
	}
	else if (first_token->type == tok_if) {
		result = create_if_statement_ast(tokens, statement_if, nullptr);
	}
	else if (first_token->type == tok_constructor) {
		result = create_constructor_declaration_ast(tokens);
	}
	else if (first_token->type == tok_for) {
		result = create_for_statement_ast(tokens);
	}
	else if (first_token->type == tok_while) {
		result = create_while_statement_ast(tokens);
	}
	else if (first_token->type == tok_vector) {
		result = create_vector_declaration_ast(tokens);
	}
	else if (first_token->identifier[0] == '\'') {
		if (first_token->identifier[2] == '\'') {
			result = new CharacterAST(first_token->identifier[1]);
		}
		else {
			// error, it is not character
		}
	}
	else if (first_token->type == tok_true || first_token->type == tok_false) {
		result = new BoolAST(first_token->identifier == L"true");
	}
	else if (first_token->type == tok_l_paren || first_token->type == tok_r_paren) {
		result = new ParenAST(first_token->identifier);
	}
	else if (first_token->type == tok_identifier) {
		result = create_identifier_ast(tokens, new IdentifierAST(first_token->identifier));
	}
	else if (first_token->type == tok_func) {
		result = create_function_declaration_ast(tokens);
	}
	else if (first_token->type == tok_var) {
		return create_variable_declaration_ast(tokens);
	}
	else if (first_token->type == tok_class || first_token->type == tok_object || first_token->type == tok_scene) {
		if (first_token->type == tok_class)
			result = create_class_declaration_ast(tokens);
		if (first_token->type == tok_object)
			result = create_object_declaration_ast(tokens);
		if (first_token->type == tok_scene)
			result = create_scene_declaration_ast(tokens);
	}
	else if (first_token->type == tok_pubilc || first_token->type == tok_private || first_token->type == tok_protected) {
		std::wstring access_modifier = first_token->identifier;

		BaseAST* _ast = parse(tokens);

		if (_ast->type == function_declaration_ast) {
			dynamic_cast<FunctionDeclarationAST*>(_ast)->access_modifier = access_modifier;
		}
		else if (_ast->type == variable_declaration_ast) {
			dynamic_cast<VariableDeclarationAST*>(_ast)->access_modifier = access_modifier;
		}
		else {
			CHESTNUT_THROW_ERROR(L"Unexpected syntax error, with access modifier, you must decalre variable or function.",
				"UNEXPECTED_TOKEN", "009", first_token->line);
		}

		result = _ast;
	}
	else if (first_token->type == tok_static) {
		BaseAST* _ast = parse(tokens);

		if (_ast->type == function_declaration_ast) {
			dynamic_cast<FunctionDeclarationAST*>(_ast)->is_static = true;
		}
		else if (_ast->type == variable_declaration_ast) {
			dynamic_cast<VariableDeclarationAST*>(_ast)->is_static = true;
		}

		result = _ast;
	}
	else if (first_token->type == tok_cast) {
		result = create_cast_ast(tokens);
	}
	else if (first_token->type == tok_return) {
		result = create_return_ast(tokens);
	}
	else if (first_token->type == tok_new) {
		result = create_new_ast(tokens);
	}
	else if (first_token->type == tok_import) {
		result = create_import_ast(tokens);
	}
	else if (first_token->type == tok_load) {
		result = create_load_ast(tokens);
	}
	else if (first_token->type == tok_font) {
		result = create_font_ast(tokens);
	}
	else if (first_token->type == tok_l_bracket) {
		result = create_array_declaration_ast(tokens);
	}
	else if (first_token->type == tok_option) {
		result = create_option_ast(first_token->identifier, tokens);
	}

	if (check_token(tokens)->type == tok_dot) {
		tokens.erase(tokens.begin());

		if (result != nullptr) {
			result->attr = parse(tokens);

			if (result->attr->type == bin_expr_ast) {
				BaseAST* lhs = result;
				result = new BinExprAST(
					dynamic_cast<BinExprAST*>(result->attr)->oper,
					lhs,
					dynamic_cast<BinExprAST*>(result->attr)->rhs);
			}
		}
	}

	if (result == nullptr) {
		Token* tok = check_token(tokens);
		CHESTNUT_THROW_ERROR(L"Unexpected token pulled : " + tok->identifier, "UNEXPECTED_TOKEN_ERROR", "0x12", tok->line);
	}

	result->line_number = first_token->line;

	if (check_token(tokens)->type == tok_semi_colon) {
		pull_token_and_expect(tokens, tok_semi_colon);
	}

	return result;
}

