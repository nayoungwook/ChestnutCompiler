#include "cn_ir_generator.h"

std::unordered_map<std::string, ClassAST*> parsed_class_data;
std::unordered_map<std::string, unsigned int> class_id;
std::unordered_map<std::string, std::unordered_map<std::string, MemberFunctionData>> member_function_data;
std::unordered_map<std::string, std::unordered_map<std::string, MemberVariableData>> member_variable_data;

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

void declare_builtin_variables() {
	Data data = { 0, "shader" };
	global_variable_symbol.insert(std::make_pair("default_shader", data));
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

scopes get_scope_of_identifier(std::string const& identifier) {
	std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();
	unsigned int id = get_local_variable_id(local_variable_symbol, identifier);

	if (identifier == "this") {
		return scope_local;
	}
	else {
		if (id == -1) {
			if (member_variable_data[current_class].find(identifier) != member_variable_data[current_class].end()) {
				return scope_class;
			}
			else if (global_variable_symbol.find(identifier) != global_variable_symbol.end()) {
				return scope_global;
			}
		}
		else {
			return scope_local;
		}
	}

	std::cout << "Failed to find identifier : " << identifier << " at this scope." << std::endl;
	exit(EXIT_FAILURE);
}

scopes get_scope_of_function(std::string const& identifier) {
	std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

	if (member_function_data[current_class].find(identifier) != member_function_data[current_class].end()) {
		return scope_class;
	}
	else if (exist_in_symbol_table(global_function_symbol, identifier)) {
		return scope_global;
	}
	else if (exist_in_symbol_table(builtin_function_symbol, identifier)) {
		return scope_local;
	}

	std::cout << "Failed to find function named : " << identifier << " at this scope." << std::endl;
	exit(EXIT_FAILURE);
}

std::string create_identifier_ir(IdentifierAST* identifier_ast) {
	std::string result = "";

	scopes scope = get_scope_of_identifier(identifier_ast->identifier);

	if (scope == scope_local) {
		result = "@LOAD_LOCAL " + std::to_string(get_local_variable_id(local_variable_symbols.top(), identifier_ast->identifier))
			+ " (" + identifier_ast->identifier + ") " + std::to_string(identifier_ast->line_number) + "\n";
	}
	else if (scope == scope_class) {
		result = "@LOAD_CLASS " + identifier_ast->identifier + " " + std::to_string(identifier_ast->line_number);
	}
	else if (scope == scope_global) {
		result = "@LOAD_GLOBAL " + identifier_ast->identifier + " " + std::to_string(identifier_ast->line_number);
	}

	result += create_attr_ir(identifier_ast, "lhs");

	return result;
}

Data get_data_of_variable(std::string const& identifier) {
	scopes scope = get_scope_of_identifier(identifier);

	if (scope == scope_local) {
		std::vector<std::unordered_map<std::string, Data>>* area = local_variable_symbols.top();
		for (int i = 0; i < area->size(); i++) {
			if (area->at(i).find(identifier) != area->at(i).end()) {
				std::unordered_map<std::string, Data>::iterator area_iterator;
				std::unordered_map<std::string, Data> found_area = area->at(i);

				for (area_iterator = found_area.begin(); area_iterator != found_area.end(); area_iterator++) {
					return area_iterator->second;
				}
			}
		}
	}
	else if (scope == scope_class) {
		Data result;
		result.id = member_variable_data[current_class][identifier].id;
		result.type = member_variable_data[current_class][identifier].type;
		return result;
	}
	else if (scope == scope_global) {
		return global_variable_symbol[identifier];
	}
}

std::string create_attr_ir(IdentifierAST* identifier_ast, std::string const& lhs_rhs) {
	IdentifierAST* searcher = identifier_ast;
	BaseAST* attr_target = identifier_ast;
	std::string result;

	while (true) {
		if (searcher->attr == nullptr) break;

		if (searcher->attr->type == bin_expr_ast) {
			if (lhs_rhs == "lhs")
				searcher = (IdentifierAST*)((BinExprAST*)searcher->attr)->lhs;
			else
				searcher = (IdentifierAST*)((BinExprAST*)searcher->attr)->rhs;
		}
		else {
			searcher = (IdentifierAST*)searcher->attr;
		}

		// case for if the target attr is identifier.
		if (attr_target->type == ast_type::identifier_ast) {
			std::string type = get_data_of_variable(((IdentifierAST*)attr_target)->identifier).type;

			MemberVariableData member_variable;

			if (member_variable_data[type].find(searcher->identifier) != member_variable_data[type].end()) {
				member_variable.id = member_variable_data[type][searcher->identifier].id + get_parent_member_variable_size(type);
				member_variable.name = searcher->identifier;
			}
			else {
				ClassAST* class_ast_searcher = parsed_class_data[type];

				while (true) {
					if (member_variable_data[class_ast_searcher->name].find(searcher->identifier) != member_variable_data[class_ast_searcher->name].end()) {
						member_variable.id
							= member_variable_data[class_ast_searcher->name][searcher->identifier].id + get_parent_member_variable_size(class_ast_searcher->name);
						member_variable.name = searcher->identifier;
					}

					if (class_ast_searcher->parent_type == "") break;
					class_ast_searcher = parsed_class_data[class_ast_searcher->parent_type];

				}
			}

			result += "@LOAD_ATTR " + std::to_string(member_variable.id)
				+ " (" + member_variable.name + ") " + std::to_string(identifier_ast->line_number) + "\n";
		}

		// add case for target attr is function ( consider return type for find variable maybe. )

		attr_target = searcher;
	}

	return result;
}

std::string create_assign_ir(BaseAST* ast, std::string& result, int indentation) {
	return "";
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

unsigned int generate_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area) {
	int result = 0;
	for (int i = 0; i < area->size(); i++) {
		result += area->at(i).size();
	}
	return result;
}

unsigned int get_parent_member_variable_size(std::string const& class_name) {
	ClassAST* class_ast_searcher = parsed_class_data[class_name];
	unsigned int parent_id_size = 0;

	while (true) {
		if (class_ast_searcher->parent_type == "") break;

		class_ast_searcher = parsed_class_data[class_ast_searcher->parent_type];

		parent_id_size += class_ast_searcher->variables.size();
	}

	return parent_id_size;
}

unsigned int get_parent_member_function_size(std::string const& class_name) {
	ClassAST* class_ast_searcher = parsed_class_data[class_name];
	unsigned int parent_id_size = 0;

	while (true) {
		if (class_ast_searcher->parent_type == "") break;

		class_ast_searcher = parsed_class_data[class_ast_searcher->parent_type];

		parent_id_size += class_ast_searcher->functions.size();
	}

	return parent_id_size;
}

const std::string create_ir(BaseAST* ast, int indentation) {

	std::string result = "";

	switch (ast->type) {

	case load_ast: {
		LoadAST* load_ast = (LoadAST*)ast;
		append_data(result, "#LOAD " + load_ast->name + " " + load_ast->path + " " + std::to_string(ast->line_number) + "\n", indentation);

		break;
	}

	case vector_declaration_ast: {

		VectorDeclarationAST* vector_declaration_ast = (VectorDeclarationAST*)ast;

		for (int i = vector_declaration_ast->elements.size() - 1; i >= 0; i--) {
			append_data(result, create_ir(vector_declaration_ast->elements[i], 0), indentation + 1);
		}

		append_data(result, "@VECTOR " + std::to_string(vector_declaration_ast->elements.size()) + " " + std::to_string(ast->line_number), indentation);

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

		append_data(result, create_identifier_ir(array_refer_ast), indentation);

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

		unsigned int id = class_id[new_ast->class_name];

		std::string parameter_count = std::to_string(new_ast->parameters.size());

		append_data(result, "@NEW " + std::to_string(id) + " (" + new_ast->class_name + ") " + parameter_count +
			" " + std::to_string(ast->line_number), indentation);
		break;
	}

	case class_ast:
	case scene_ast:
	case object_ast:
	{
		ClassAST* class_ast = ((ClassAST*)ast);
		scopes backup_scope = current_scope;
		current_scope = scope_class;
		current_class = class_ast->name;

		std::string parent_type;
		if (class_ast->parent_type == "")
			parent_type = "-1";
		else {
			parent_type = std::to_string(class_id[class_ast->parent_type]);
		}

		current_class = class_ast->name;

		std::string object_type;

		if (ast->type == ast_type::class_ast)
			object_type = "CLASS";
		else if (ast->type == ast_type::scene_ast)
			object_type = "SCENE";
		else if (ast->type == ast_type::object_ast)
			object_type = "OBJECT";

		create_scope();
		unsigned int id = class_id[class_ast->name];
		std::string line = object_type + " " + std::to_string(id) + " (" + class_ast->name + ") " + parent_type + " {";
		append_data(result, line, indentation);

		create_scope();

		line = "$INITIALIZE 0 (constructor) default void {";
		append_data(result, line, indentation);

		for (int i = 0; i < class_ast->variables.size(); i++) {
			append_data(result, create_ir(class_ast->variables[i], indentation), 0);
		}

		line = "}";
		append_data(result, line, indentation);

		destroy_scope();

		for (int i = 0; i < class_ast->functions.size(); i++) {
			append_data(result, create_ir(class_ast->functions[i], indentation), 0);
		}

		for (ConstructorDeclarationAST* constructor_declaration : class_ast->constructor) {
			append_data(result, create_ir(constructor_declaration, indentation), 0);
		}

		line = "}";

		destroy_scope();

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

		create_scope();

		line += "{";

		append_data(result, line, indentation);

		for (int i = 0; i < constructor_declaration_ast->body.size(); i++) {
			append_data(result, create_ir(constructor_declaration_ast->body[i], indentation), 0);
		}

		append_data(result, "}", indentation);

		destroy_scope();

		current_scope = scope_global;
		break;
	}

	case function_declaration_ast: {
		FunctionDeclarationAST* function_declaration_ast = ((FunctionDeclarationAST*)ast);

		std::string function_name = function_declaration_ast->function_name;

		unsigned int id = 0;

		switch (current_scope) {
		case scope_global:
			id = (unsigned int)global_function_symbol.size();
			global_function_symbol.insert(std::make_pair(function_declaration_ast->function_name,
				global_function_symbol.size()));
			break;
		case scope_class:

			break;
		}

		if (current_scope == scope_global) {
			global_function_symbol.insert(std::make_pair(function_name, global_function_symbol.size()));
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

		append_data(result, create_identifier_ir(_identifier_ast), indentation);
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

	case while_statement_ast: {

		local_variable_symbols.top()->push_back({});

		WhileStatementAST* while_statement_ast = (WhileStatementAST*)(ast);

		label_id++;
		int begin_id = label_id;
		label_id++;
		int condition_id = label_id;

		append_data(result, "@GOTO " + integer_to_hex(condition_id) + " " + std::to_string(ast->line_number), indentation);

		append_data(result, "@LABEL " + integer_to_hex(begin_id) + " " + std::to_string(ast->line_number), 0);

		for (int i = 0; i < while_statement_ast->body.size(); i++) {
			append_data(result, create_ir(while_statement_ast->body[i], indentation), 0);
		}

		append_data(result, "@LABEL " + integer_to_hex(condition_id) + " " + std::to_string(ast->line_number), 0);

		append_data(result, create_ir(while_statement_ast->condition, indentation), indentation);

		append_data(result, "@FOR " + integer_to_hex(begin_id) + " " + std::to_string(ast->line_number), indentation);

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

			if (variable_declaration_ast->declarations[i] != nullptr) {
				std::string param = create_ir(variable_declaration_ast->declarations[i], 0);
				append_data(result, param, indentation);
			}
			else {
				if (variable_declaration_ast->var_types[i] == "number") {
					append_data(result, "@PUSH_NUMBER 0" + std::to_string(ast->line_number), indentation);
				}
				else {
					append_data(result, "@PUSH_NULL " + std::to_string(ast->line_number), indentation);
				}
			}

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
				id = generate_local_variable_id(local_variable_symbol);

				local_variable_symbol->at(local_variable_symbol->size() - 1).insert(std::make_pair(variable_declaration_ast->names[i],
					Data{ id, variable_declaration_ast->var_types[i] }));

				break;
			}
			case scope_class:
				store_type = "@STORE_CLASS";

				id = get_parent_member_variable_size(current_class) + member_variable_data[current_class][variable_declaration_ast->names[i]].id;

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
		else if (bin_expr_ast->oper == "+=" ||
			bin_expr_ast->oper == "-=" ||
			bin_expr_ast->oper == "*=" ||
			bin_expr_ast->oper == "/=" ||
			bin_expr_ast->oper == "^=" ||
			bin_expr_ast->oper == "%=") {

			std::string lhs = create_ir(bin_expr_ast->lhs, indentation);
			append_data(result, lhs, 0);

			std::string rhs = create_ir(bin_expr_ast->rhs, indentation);
			append_data(result, rhs, 0);

			std::string oper = "";

			if (bin_expr_ast->oper == "+=")
				oper = "@ADD";
			else if (bin_expr_ast->oper == "-=")
				oper = "@SUB";
			else if (bin_expr_ast->oper == "*=")
				oper = "@MUL";
			else if (bin_expr_ast->oper == "/=")
				oper = "@DIV";
			else if (bin_expr_ast->oper == "%=")
				oper = "@MOD";
			else if (bin_expr_ast->oper == "^=")
				oper = "@POW";

			oper += " " + std::to_string(ast->line_number);
			append_data(result, oper, indentation);

			bin_expr_ast->rhs = nullptr;
			create_assign_ir(bin_expr_ast, result, indentation);
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

		if (function_name == "super") {
			std::string line = "@SUPER_CALL " + std::to_string(function_call_ast->parameters.size())
				+ " " + std::to_string(ast->line_number);

			for (int i = function_call_ast->parameters.size() - 1; i >= 0; i--) {
				std::string param = create_ir(function_call_ast->parameters[i], 0);
				append_data(result, param, indentation);
			}

			append_data(result, line, 1);

			break;
		}

		unsigned int function_id = -1;
		std::string call_type = "";

		scopes scope = get_scope_of_function(function_name);

		if (scope == scope_class) {
			function_id = member_function_data[current_class][function_name].id;

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