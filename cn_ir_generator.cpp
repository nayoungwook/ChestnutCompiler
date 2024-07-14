#include "cn_ir_generator.h"

inline bool exist_in_symbol_table(std::unordered_map<std::string, unsigned int> area, std::string const& name) {
	return area.find(name) != area.end();
}

void append_data(std::string& target, std::string content, int indentation) {

	for (int i = 0; i < indentation; i++) {
		target += "    ";
	}

	target += content + "\n";
}

void declare_builtin_functions() {
	builtin_function_symbol.insert(std::make_pair("print", 0));
	builtin_function_symbol.insert(std::make_pair("window", 1));
	builtin_function_symbol.insert(std::make_pair("load_scene", 2));
	builtin_function_symbol.insert(std::make_pair("image", 3));
}

void create_scope() {
	std::vector<std::unordered_map<std::string, Data>>* temp = new std::vector<std::unordered_map<std::string, Data>>;
	temp->push_back({});
	local_variable_symbols.push(temp);
}

void destroy_scope() {
	local_variable_symbols.pop();
}

const std::string integer_to_hex(int i) {
	std::ostringstream ss;
	ss << "0x" << std::hex << i;
	std::string result = ss.str();
	return result;
}

Data* get_local_variable(std::vector<std::unordered_map<std::string, Data>>* area, std::string const& obj_identifier) {
	for (int i = 0; i < area->size(); i++) {
		if (area->at(i).find(obj_identifier) != area->at(i).end()) {
			return &area->at(i).find(obj_identifier)->second;
		}
	}
	return nullptr;
}

unsigned int get_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area, std::string const& obj_identifier) {
	int result = 0;
	for (int i = 0; i < area->size(); i++) {
		if (area->at(i).find(obj_identifier) != area->at(i).end()) {
			std::unordered_map<std::string, Data>::iterator area_iterator;
			std::unordered_map<std::string, Data> found_area = area->at(i);

			for (area_iterator = found_area.begin(); area_iterator != found_area.end(); area_iterator++) {
				if (area_iterator->first == obj_identifier) break;

				result++;
			}
			return result;
		}
		result += area->at(i).size();
	}
	return result;
}

unsigned int get_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area) {
	int result = 0;
	for (int i = 0; i < area->size(); i++) {
		result += area->at(i).size();
	}
	return result;
}

Data* create_identifier_ast(std::string const& identifier, std::string& result, int line_number, int indentation) {

	std::string load_type = "";
	unsigned int id = 0;

	std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

	Data* data = get_local_variable(local_variable_symbol, identifier);

	load_type = "@LOAD_LOCAL";
	id = get_local_variable_id(local_variable_symbol, identifier);

	if (data == nullptr) {
		if (exist_in_symbol_table(class_member_variables->find(current_class)->second, identifier)) {
			std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
			id = _class->find(identifier)->second;
			load_type = "@LOAD_CLASS";
		}
		else if (global_variable_symbol.find(identifier) != global_variable_symbol.end()) {
			data = &global_variable_symbol[identifier];
			id = data->id;
			load_type = "@LOAD_GLOBAL";
		}
	}

	std::string line = load_type + " " + std::to_string(id)
		+ " (" + identifier + ") " + std::to_string(line_number);

	append_data(result, line, indentation);
	return data;
}

void create_assign_ir(BaseAST* ast, std::string& result, int indentation) {
	BinExprAST* bin_expr_ast = ((BinExprAST*)ast);

	std::string rhs = create_ir(bin_expr_ast->rhs, indentation);
	append_data(result, rhs, indentation);

	std::string lhs_data;

	BaseAST* lhs = bin_expr_ast->lhs; // base lhs
	BaseAST* seracher = lhs; // searcher ast
	BaseAST* object_ast = nullptr; // target object ast
	BaseAST* last_ast = lhs; // last ast

	while (seracher != nullptr && seracher->attr != nullptr) {
		last_ast = seracher->attr;

		if (last_ast->type == ast_type::bin_expr_ast)
			last_ast = ((BinExprAST*)last_ast)->lhs;

		object_ast = seracher;

		if (last_ast->attr == nullptr) {
			seracher->attr = nullptr;
			break;
		}

		seracher = seracher->attr;

		if (seracher->type == ast_type::bin_expr_ast)
			seracher = ((BinExprAST*)seracher)->lhs;
	}

	if (last_ast == lhs) {
		std::string lhs_data = create_ir(seracher, 0);
	}
	else {
		append_data(result, create_ir(lhs, 0), indentation);

		if (last_ast == nullptr)
			return;

		std::string store_type = "";
		std::string identifier = ((IdentifierAST*)last_ast)->identifier;
		std::string obj_identifier = ((IdentifierAST*)object_ast)->identifier;
		std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

		Data* data = get_local_variable(local_variable_symbol, obj_identifier);

		if (data == nullptr) {
			if (exist_in_symbol_table(class_member_variables->find(current_class)->second, obj_identifier)) {
				std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
			}
			else if (global_variable_symbol.find(obj_identifier) != global_variable_symbol.end()) {
				data = &global_variable_symbol[obj_identifier];
			}
		}

		if (data == nullptr) {
			std::cout << obj_identifier << " doesn\'t contain variable : " << identifier << std::endl;
			exit(EXIT_FAILURE);
		}

		std::unordered_map<std::string, unsigned int> class_data = class_member_variables->find(data->type)->second;

		lhs_data = "@STORE_ATTR " + std::to_string(class_data[identifier])
			+ " (" + identifier + ") " + std::to_string(ast->line_number);

	}

	append_data(result, lhs_data, indentation);
}

const std::string create_ir(BaseAST* ast, int indentation) {
	std::string result = "";

	switch (ast->type) {

	case load_ast: {
		LoadAST* load_ast = (LoadAST*)ast;
		append_data(result, "#LOAD " + load_ast->name + " " + load_ast->path + " " + std::to_string(ast->line_number) + "\n", indentation);

		break;
	}

	case array_declaration_ast: {
		ArrayDeclarationAST* array_ast = (ArrayDeclarationAST*)ast;

		for (int i = array_ast->elements.size() - 1; i >= 0; i--) {
			append_data(result, create_ir(array_ast->elements[i], 0), indentation + 1);
		}

		append_data(result, "@ARRAY " + std::to_string(array_ast->elements.size()) + " " + std::to_string(ast->line_number), indentation);

		break;
	}

	case array_refer_ast: {
		ArrayReferAST* array_refer_ast = (ArrayReferAST*)ast;

		create_identifier_ast(array_refer_ast->identifier, result, array_refer_ast->line_number, indentation);

		for (int i = 0; i < array_refer_ast->indexes.size(); i++) {
			append_data(result, create_ir(array_refer_ast->indexes[i], indentation), indentation);
			append_data(result, "@ARRAY_GET " + std::to_string(ast->line_number), indentation);
		}

		break;
	}

	case new_ast: {
		NewAST* new_ast = ((NewAST*)ast);

		for (int i = new_ast->parameters.size() - 1; i >= 0; i--) {
			std::string param = create_ir(new_ast->parameters[i], 0);
			append_data(result, param, indentation);
		}

		unsigned int id = class_symbol[new_ast->class_name];

		std::string parameter_count = std::to_string(new_ast->parameters.size());

		append_data(result, "@NEW " + std::to_string(id) + " (" + new_ast->class_name + ") " + parameter_count +
			" " + std::to_string(ast->line_number), indentation);
		break;
	}

	case class_ast:
	case scene_ast:
	{
		ClassAST* class_ast = ((ClassAST*)ast);
		scopes backup_scope = current_scope;
		current_scope = scope_class;

		unsigned int id = class_symbol.size();
		class_symbol.insert(std::make_pair(class_ast->name, id));

		std::string parent_type;
		if (class_ast->parent_type == "")
			parent_type = "-1";
		else {
			parent_type = std::to_string(class_symbol[class_ast->parent_type]);
		}


		std::unordered_map<std::string, unsigned int> temp = {};
		class_member_functions->insert(std::make_pair(class_ast->name, temp));
		class_member_variables->insert(std::make_pair(class_ast->name, temp));

		current_class = class_ast->name;

		std::string object_type;

		if (ast->type == ast_type::class_ast)
			object_type = "CLASS";
		else if (ast->type == ast_type::scene_ast)
			object_type = "SCENE";
		else if (ast->type == ast_type::object_ast)
			object_type = "OBJECT";

		std::string line = object_type + " " + std::to_string(id) + " (" + class_ast->name + ") " + parent_type + " {";
		append_data(result, line, indentation);

		line = "$INITIALIZE 0 (constructor) default void {";
		append_data(result, line, indentation);

		for (int i = 0; i < class_ast->variables.size(); i++) {
			append_data(result, create_ir(class_ast->variables[i], indentation), 0);
		}

		line = "}";

		append_data(result, line, indentation);

		for (int i = 0; i < class_ast->functions.size(); i++) {
			append_data(result, create_ir(class_ast->functions[i], indentation), 0);
		}

		for (ConstructorDeclarationAST* constructor_declaration : class_ast->constructor) {
			append_data(result, create_ir(constructor_declaration, indentation), 0);
		}

		line = "}";
		append_data(result, line, indentation);

		current_scope = backup_scope;

		break;
	}

	case constructor_declaration_ast: {
		ConstructorDeclarationAST* constructor_declaration_ast = (ConstructorDeclarationAST*)ast;

		std::string line = "$CONSTRUCTOR 0 (constructor) " + constructor_declaration_ast->access_modifier + " " + constructor_declaration_ast->return_type + " ";

		current_scope = scope_local;

		for (int i = 0; i < constructor_declaration_ast->parameter.size(); i++) {
			VariableDeclarationAST* param = constructor_declaration_ast->parameter[i];

			line += param->names[0] + " " + param->var_types[0] + " ";
		}
		line += "{";

		append_data(result, line, indentation);

		for (int i = 0; i < constructor_declaration_ast->body.size(); i++) {
			append_data(result, create_ir(constructor_declaration_ast->body[i], indentation), 0);
		}

		append_data(result, "}", indentation);

		current_scope = scope_global;
		break;
	}

	case function_declaration_ast: {
		FunctionDeclarationAST* function_declaration_ast = ((FunctionDeclarationAST*)ast);

		std::string function_name = function_declaration_ast->function_name;

		if (current_scope == scope_global) {
			global_function_symbol.insert(std::make_pair(function_name, global_function_symbol.size()));
		}

		unsigned int id = 0;

		switch (current_scope) {
		case scope_global:
			id = (unsigned int)global_function_symbol.size();
			global_function_symbol.insert(std::make_pair(function_declaration_ast->function_name,
				global_function_symbol.size()));
			break;
		case scope_class:
			std::unordered_map<std::string, unsigned int>* _class = &class_member_functions->find(current_class)->second;
			id = (unsigned int)_class->size();
			_class->insert(std::make_pair(function_name, id));
			break;
		}

		scopes backup_scope = current_scope;
		current_scope = scope_local;

		create_scope();

		std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

		std::string line = "FUNC " + std::to_string(id) + " (" + function_name + ") ";

		line += function_declaration_ast->access_modifier + " ";

		for (int i = 0; i < function_declaration_ast->parameter.size(); i++) {
			line += function_declaration_ast->parameter[i]->var_types[0] + " ";

			local_variable_symbol->at(local_variable_symbol->size() - 1).insert(
				std::make_pair(function_declaration_ast->parameter[i]->names[i],
					Data{ (unsigned int)local_variable_symbol->size(), function_declaration_ast->parameter[i]->var_types[i] }
				));
		}

		line += function_declaration_ast->return_type + " {\n";

		for (BaseAST* ast : function_declaration_ast->body) {
			std::string body = create_ir(ast, indentation + 1);
			append_data(line, body, 0);
		}

		line += "}";

		append_data(result, line, 0);

		destroy_scope();

		current_scope = backup_scope;

		break;
	};

	case string_literal_ast: {
		StringLiteralAST* string_literal_ast = ((StringLiteralAST*)ast);
		std::string str_literal = string_literal_ast->str_literal;

		std::string line = "@PUSH_STRING " + str_literal + " " + std::to_string(ast->line_number);

		append_data(result, line, 0);
		break;
	};

	case bool_ast: {
		BoolAST* bool_ast = (BoolAST*)ast;

		std::string bool_data = (bool_ast->bool_data ? "true" : "false");
		std::string line = "@PUSH_BOOL " + bool_data + " " + std::to_string(ast->line_number);

		append_data(result, line, 0);

		break;
	}

	case identifier_ast: {
		IdentifierAST* _identifier_ast = ((IdentifierAST*)ast);
		std::string identifier = _identifier_ast->identifier;

		Data* data = create_identifier_ast(identifier, result, _identifier_ast->line_number, indentation);

		BaseAST* searcher = _identifier_ast;

		while (searcher->attr != nullptr) {
			if (searcher->attr->type == identifier_ast) {
				IdentifierAST* ident_ast = (IdentifierAST*)searcher->attr;
				std::unordered_map<std::string, unsigned int> _class = class_member_variables->find(data->type)->second;
				unsigned int id = _class.find(ident_ast->identifier)->second;

				std::string _attr = "@LOAD_ATTR " + std::to_string(id)
					+ " (" + ident_ast->identifier + ") " + std::to_string(ast->line_number);

				append_data(result, _attr, indentation);
			}
			else if (searcher->attr->type == function_call_ast) {
				FunctionCallAST* func_call = (FunctionCallAST*)searcher->attr;
				std::unordered_map<std::string, unsigned int> _class = class_member_functions->find(data->type)->second;
				unsigned int id = _class.find(func_call->function_name)->second;

				for (int i = func_call->parameters.size() - 1; i >= 0; i--) {
					std::string param = create_ir(func_call->parameters[i], 0);
					append_data(result, param, indentation);
				}

				std::string _attr = "@CALL_ATTR " + std::to_string(id)
					+ " (" + func_call->function_name + ") " + std::to_string(func_call->parameters.size())
					+ " " + std::to_string(ast->line_number);


				append_data(result, _attr, indentation);
			}
			searcher = searcher->attr;
		}

		break;
	};

	case number_ast: {
		NumberAST* number_ast = ((NumberAST*)ast);

		std::string number = number_ast->number_string;

		std::string line = "@PUSH_NUMBER " + number + " " + std::to_string(ast->line_number);

		append_data(result, line, indentation);
		break;
	}

	case if_statement_ast: {
		IfStatementAST* if_statement_ast = (IfStatementAST*)ast;

		if (if_statement_ast->statement_type != statement_else)
			append_data(result, create_ir(if_statement_ast->condition, indentation), 0);

		// store block end label count of if statement 
		label_id++;
		int end_label_count = label_id;

#pragma region IfBlock

		label_id++;
		int block_id = label_id;
		if (if_statement_ast->statement_type != statement_else) {
			append_data(result, "@IF " + integer_to_hex(block_id) + " " + std::to_string(ast->line_number), indentation);
		}

		local_variable_symbols.top()->push_back({});

		for (int i = 0; i < if_statement_ast->body.size(); i++) {
			append_data(result, create_ir(if_statement_ast->body[i], indentation), 0);
		}

		local_variable_symbols.top()->erase(local_variable_symbols.top()->begin() + local_variable_symbols.top()->size() - 1);

		// as the if statement ends terminate the entire statement
		append_data(result, "@GOTO " + integer_to_hex(end_label_count) + " " + std::to_string(ast->line_number), indentation);

		if (if_statement_ast->statement_type != statement_else) {
			append_data(result, "@LABEL " + integer_to_hex(block_id) + " " + std::to_string(ast->line_number), indentation);
		}

#pragma endregion 

		for (int i = 0; i < if_statement_ast->additional_statements.size(); i++) {
			append_data(result, create_ir(if_statement_ast->additional_statements[i], indentation), 0);
		}

		// end the if statement label
		append_data(result, "@LABEL " + integer_to_hex(end_label_count) + " " + std::to_string(ast->line_number), indentation);
		break;
	}

	case for_statement_ast: {
		ForStatementAST* for_statement_ast = ((ForStatementAST*)ast);

		local_variable_symbols.top()->push_back({});

		append_data(result, create_ir(for_statement_ast->init, indentation), 0);

		label_id++;
		int end_label_id = label_id;
		label_id++;
		int begin_label_id = label_id;

		append_data(result, "@GOTO " + integer_to_hex(end_label_id) + " " + std::to_string(ast->line_number), indentation);

		append_data(result, "@LABEL " + integer_to_hex(begin_label_id) + " " + std::to_string(ast->line_number), indentation);

		for (int i = 0; i < for_statement_ast->body.size(); i++) {
			append_data(result, create_ir(for_statement_ast->body[i], indentation), 0);
		}

		append_data(result, create_ir(for_statement_ast->step, indentation), 0);

		append_data(result, "@LABEL " + integer_to_hex(end_label_id) + " " + std::to_string(ast->line_number), indentation);

		append_data(result, create_ir(for_statement_ast->condition, indentation), indentation);

		append_data(result, "@FOR " + integer_to_hex(begin_label_id) + " " + std::to_string(ast->line_number), indentation);

		local_variable_symbols.top()->erase(local_variable_symbols.top()->begin() + local_variable_symbols.top()->size() - 1);

		break;
	}

	case return_ast: {
		ReturnAST* return_ast = ((ReturnAST*)ast);
		std::string expr = create_ir(return_ast->expression, indentation);
		append_data(result, expr, indentation);

		std::string line = "@RET " + std::to_string(ast->line_number);
		append_data(result, line, indentation);
		break;
	}

	case variable_declaration_ast: {
		VariableDeclarationAST* variable_declaration_ast = ((VariableDeclarationAST*)ast);

		for (int i = 0; i < variable_declaration_ast->var_count; i++) {
			std::string param = create_ir(variable_declaration_ast->declarations[i], 0);
			append_data(result, param, indentation);

			std::string store_type = "";
			unsigned int id = 0;

			switch (current_scope) {
			case scope_global:
				store_type = "@STORE_GLOBAL";
				id = (unsigned int)global_variable_symbol.size();
				global_variable_symbol.insert(std::make_pair(variable_declaration_ast->names[i],
					Data{ id, variable_declaration_ast->var_types[i] }
				));
				break;
			case scope_local: {
				std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

				store_type = "@STORE_LOCAL";
				id = get_local_variable_id(local_variable_symbol);

				local_variable_symbol->at(local_variable_symbol->size() - 1).insert(std::make_pair(variable_declaration_ast->names[i],
					Data{ id, variable_declaration_ast->var_types[i] }));

				break;
			}
			case scope_class:
				store_type = "@STORE_CLASS";

				std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
				id = (unsigned int)_class->size();
				_class->insert(std::make_pair(variable_declaration_ast->names[i], id));
				break;
			}

			std::string line = store_type + " " + std::to_string(id)
				+ " (" + variable_declaration_ast->names[i] + ") " + std::to_string(ast->line_number);

			append_data(result, line, indentation);
		}

		break;
	};

	case bin_expr_ast: {
		BinExprAST* bin_expr_ast = ((BinExprAST*)ast);

		if (bin_expr_ast->oper == "=") {
			create_assign_ir(ast, result, indentation);
		}
		else if (bin_expr_ast->oper == "++" || bin_expr_ast->oper == "--") {
			std::string lhs = create_ir(bin_expr_ast->lhs, indentation);
			append_data(result, lhs, 0);

			std::string oper = "";

			if (bin_expr_ast->oper == "++")
				oper = "@INCRE";
			else if (bin_expr_ast->oper == "--")
				oper = "@DECRE";

			oper += " " + std::to_string(ast->line_number);

			append_data(result, oper, indentation);
		}
		else if (bin_expr_ast->oper == ">" ||
			bin_expr_ast->oper == "<" ||
			bin_expr_ast->oper == ">=" ||
			bin_expr_ast->oper == "<=" ||
			bin_expr_ast->oper == "==" ||
			bin_expr_ast->oper == "!=" ||
			bin_expr_ast->oper == "||" ||
			bin_expr_ast->oper == "&&") {

			std::string lhs = create_ir(bin_expr_ast->lhs, indentation);
			append_data(result, lhs, 0);

			std::string rhs = create_ir(bin_expr_ast->rhs, indentation);
			append_data(result, rhs, 0);

			std::string oper = "";

			if (bin_expr_ast->oper == ">")
				oper = "@LESSER";
			else if (bin_expr_ast->oper == "<")
				oper = "@GREATER";
			else if (bin_expr_ast->oper == ">=")
				oper = "@EQ_LESSER";
			else if (bin_expr_ast->oper == "<=")
				oper = "@EQ_GREATER";
			else if (bin_expr_ast->oper == "==")
				oper = "@EQUAL";
			else if (bin_expr_ast->oper == "!=")
				oper = "@NOT_EQUAL";
			else if (bin_expr_ast->oper == "||")
				oper = "@OR";
			else if (bin_expr_ast->oper == "&&")
				oper = "@AND";

			oper += " " + std::to_string(ast->line_number);

			append_data(result, oper, indentation);
		}
		else {
			std::string oper = "";

			std::string lhs = create_ir(bin_expr_ast->lhs, indentation);
			append_data(result, lhs, 0);

			std::string rhs = create_ir(bin_expr_ast->rhs, indentation);
			append_data(result, rhs, 0);

			if (bin_expr_ast->oper == "+")
				oper = "@ADD";
			else if (bin_expr_ast->oper == "-")
				oper = "@SUB";
			else if (bin_expr_ast->oper == "*")
				oper = "@MUL";
			else if (bin_expr_ast->oper == "/")
				oper = "@DIV";
			else if (bin_expr_ast->oper == "%")
				oper = "@MOD";
			else if (bin_expr_ast->oper == "^")
				oper = "@POW";

			oper += " " + std::to_string(ast->line_number);

			append_data(result, oper, 0);
		}

		break;
	}

	case function_call_ast: {
		FunctionCallAST* function_call_ast = ((FunctionCallAST*)ast);
		std::string function_name = function_call_ast->function_name;

		unsigned int function_id = -1;
		std::string call_type = "";

		if (class_member_functions->find(current_class) != class_member_functions->end() && exist_in_symbol_table(class_member_functions->find(current_class)->second, function_name)) {
			std::unordered_map<std::string, unsigned int>* _class = &class_member_functions->find(current_class)->second;
			function_id = _class->find(function_name)->second;

			call_type = "@CALL_CLASS";
		}
		else if (exist_in_symbol_table(global_function_symbol, function_name)) {
			function_id = global_function_symbol[function_name];
			call_type = "@CALL_GLOBAL";
		}
		else if (exist_in_symbol_table(builtin_function_symbol, function_name)) {
			function_id = builtin_function_symbol[function_name];
			call_type = "@CALL_BUILTIN";
		}
		else {
			std::cout << "Error! cannot find function : " << function_name << std::endl;
			exit(EXIT_FAILURE);
		}

		std::string line = call_type + " " + std::to_string(function_id)
			+ " (" + function_name + ") " + std::to_string(function_call_ast->parameters.size())
			+ " " + std::to_string(ast->line_number);

		for (int i = function_call_ast->parameters.size() - 1; i >= 0; i--) {
			std::string param = create_ir(function_call_ast->parameters[i], 0);
			append_data(result, param, indentation);
		}

		append_data(result, line, 1);

		break;
	};

	}

	return result;
}