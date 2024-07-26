#include "cn_ir_generator.h"

std::unordered_map<std::string, ClassAST*> parsed_class_data;

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

Data* get_local_variable(std::vector<std::unordered_map<std::string, Data>>* area, std::string const& obj_identifier) {
	for (int i = 0; i < area->size(); i++) {
		if (area->at(i).find(obj_identifier) != area->at(i).end()) {
			return &area->at(i).find(obj_identifier)->second;
		}
	}
	return nullptr;
}

Data* get_data(std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol, std::string const& identifier) {
	Data* data = get_local_variable(local_variable_symbol, identifier);

	scopes result = scopes::scope_local;
	unsigned int id = get_local_variable_id(local_variable_symbol, identifier);

	if (data == nullptr) {
		if (exist_in_symbol_table(class_member_variables->find(current_class)->second, identifier)) {
			std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
			std::unordered_map<std::string, std::string>* _class_type = &class_member_variables_type->find(current_class)->second;
			id = _class->find(identifier)->second;

			data = new Data;
			data->type = _class_type->find(identifier)->second;
			data->id = id;

			result = scopes::scope_class;
		}
		else if (global_variable_symbol.find(identifier) != global_variable_symbol.end()) {
			data = &global_variable_symbol[identifier];
			id = data->id;
			result = scopes::scope_global;
		}
	}

	return data;
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

std::pair<scopes, Data*> get_memory_scope(std::string const& identifier) {

	std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

	Data* data = get_local_variable(local_variable_symbol, identifier);

	scopes result = scopes::scope_local;
	unsigned int id = get_local_variable_id(local_variable_symbol, identifier);

	if (data == nullptr) {
		if (exist_in_symbol_table(class_member_variables->find(current_class)->second, identifier)) {
			std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
			std::unordered_map<std::string, std::string>* _class_type = &class_member_variables_type->find(current_class)->second;
			id = _class->find(identifier)->second;

			data = new Data;
			data->type = _class_type->find(identifier)->second;
			data->id = id;

			result = scopes::scope_class;
		}
		else if (global_variable_symbol.find(identifier) != global_variable_symbol.end()) {
			data = &global_variable_symbol[identifier];
			id = data->id;
			result = scopes::scope_global;
		}
	}

	return std::make_pair(result, data);
}

Data* create_identifier_ast(BaseAST* identifier_ast, std::string const& identifier, std::string& result, int line_number, int indentation) {

	std::string load_type = "";
	unsigned int id = 0;

	std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

	Data* data = nullptr;
	if (identifier == "this") {
		data = new Data;
		data->id = current_class_id;
		data->type = current_class;

		std::string line = "@PUSH_THIS " + std::to_string(line_number);

		append_data(result, line, indentation);
	}
	else {
		data = get_local_variable(local_variable_symbol, identifier);

		load_type = "@LOAD_LOCAL";
		id = get_local_variable_id(local_variable_symbol, identifier);

		if (data == nullptr) {
			if (exist_in_symbol_table(class_member_variables->find(current_class)->second, identifier)) {
				std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
				std::unordered_map<std::string, std::string>* _class_type = &class_member_variables_type->find(current_class)->second;
				id = _class->find(identifier)->second;

				data = new Data;
				data->type = _class_type->find(identifier)->second;
				data->id = id;

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
	}

	BaseAST* searcher = identifier_ast;

	while (searcher->attr != nullptr) {
		if (searcher->attr->type == ast_type::bin_expr_ast)
			searcher->attr = ((BinExprAST*)searcher->attr)->lhs;

		if (searcher->attr->type == ast_type::identifier_ast) {
			IdentifierAST* ident_ast = (IdentifierAST*)searcher->attr;
			unsigned int id = 0;
			std::string _attr;

			if (class_member_variables->find(data->type) != class_member_variables->end()) { // case for attr of class memory
				std::unordered_map<std::string, unsigned int> _class = class_member_variables->find(data->type)->second;
				id = _class.find(ident_ast->identifier)->second;

				_attr = "@LOAD_ATTR " + std::to_string(id)
					+ " (" + ident_ast->identifier + ") " + std::to_string(identifier_ast->line_number);
			}
			else { // case for @1 ( vector data )
				std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();
				Data* data = get_local_variable(local_variable_symbol, identifier);

				if (ident_ast->identifier == "x") id = 0;
				if (ident_ast->identifier == "y") id = 1;
				if (ident_ast->identifier == "z") id = 2;

				if (data != nullptr) {
					if (data->type == "vector") {
						_attr = "@LOAD_ATTR " + std::to_string(id)
							+ " (" + ident_ast->identifier + ") " + std::to_string(identifier_ast->line_number);

					}
				}
			}
			append_data(result, _attr, indentation);
		}
		else if (searcher->attr->type == function_call_ast) {
			FunctionCallAST* func_call = (FunctionCallAST*)searcher->attr;
			IdentifierAST* ident_ast = (IdentifierAST*)searcher;

			Data* data = get_data(local_variable_symbol, ident_ast->identifier);

			std::unordered_map<std::string, unsigned int> _class = class_member_functions->find(data->type)->second;
			unsigned int id = _class.find(func_call->function_name)->second;

			for (int i = func_call->parameters.size() - 1; i >= 0; i--) {
				std::string param = create_ir(func_call->parameters[i], 0);
				append_data(result, param, indentation);
			}

			std::string _attr = "@CALL_ATTR " + std::to_string(id)
				+ " (" + func_call->function_name + ") " + std::to_string(func_call->parameters.size())
				+ " " + std::to_string(identifier_ast->line_number);


			append_data(result, _attr, indentation);
		}
		searcher = searcher->attr;
	}

	return data;
}

std::string get_store_type(unsigned int& id, BaseAST* last_ast) {
	std::string store_type = "";
	std::string identifier = ((IdentifierAST*)last_ast)->identifier;
	if (last_ast->type == identifier_ast) {
		std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

		Data* data = get_local_variable(local_variable_symbol, identifier);

		store_type = "@STORE_LOCAL";
		id = get_local_variable_id(local_variable_symbol, identifier);

		if (data == nullptr) {
			if (exist_in_symbol_table(class_member_variables->find(current_class)->second, identifier)) {
				std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
				id = _class->find(identifier)->second;
				store_type = "@STORE_CLASS";
			}
			else if (global_variable_symbol.find(identifier) != global_variable_symbol.end()) {
				data = &global_variable_symbol[identifier];
				id = data->id;
				store_type = "@STORE_GLOBAL";
			}
		}
	}

	return store_type;
}

void create_assign_ir(BaseAST* ast, std::string& result, int indentation) {
	BinExprAST* bin_expr_ast = ((BinExprAST*)ast);

	if (bin_expr_ast->rhs != nullptr) { // if there's no rhs, then it means that rhs already created.
		std::string rhs = create_ir(bin_expr_ast->rhs, indentation);
		append_data(result, rhs, indentation);
	}

	BaseAST* searcher = ((BinExprAST*)ast)->lhs;
	BaseAST* last_ast = ((BinExprAST*)ast)->lhs;
	BaseAST* attr_target_ast = ((BinExprAST*)ast)->lhs;

	while (searcher->attr != nullptr) {
		if (searcher->attr->attr == nullptr) {
			if (searcher->attr->type == ast_type::bin_expr_ast)
				last_ast = ((BinExprAST*)searcher->attr)->lhs;
			else
				last_ast = searcher->attr;
			searcher->attr = nullptr;
			break;
		}

		searcher = ((BinExprAST*)searcher->attr)->lhs;
		attr_target_ast = searcher;
	}

	if (((BinExprAST*)ast)->lhs == last_ast) { // single node.
		std::string identifier = ((IdentifierAST*)searcher)->identifier;
		std::pair<scopes, Data*> scope_result = get_memory_scope(identifier);
		std::string store_type = get_store_type(scope_result.second->id, searcher);

		std::string line = store_type + " " + std::to_string(scope_result.second->id)
			+ " (" + identifier + ") " + std::to_string(ast->line_number);

		append_data(result, line, indentation);
	}

	if (((BinExprAST*)ast)->lhs != last_ast) { // attr, multiple nodes
		Data* data = create_identifier_ast(((BinExprAST*)ast)->lhs, ((IdentifierAST*)((BinExprAST*)ast)->lhs)->identifier, result, ast->line_number, indentation);

		if (data->type == "vector") { // vector attr
			std::string attr_identifier = ((IdentifierAST*)last_ast)->identifier;
			unsigned int id = 0;

			if (attr_identifier == "x") id = 0;
			if (attr_identifier == "y") id = 1;
			if (attr_identifier == "z") id = 2;

			std::string line = "@STORE_ATTR " + std::to_string(id)
				+ " (" + attr_identifier + ") " + std::to_string(ast->line_number);
			append_data(result, line, indentation);
		}
		else {
			std::unordered_map<std::string, unsigned int> member_variables = class_member_variables->find(data->type)->second;
			std::string attr_identifier = ((IdentifierAST*)last_ast)->identifier;

			unsigned int id = member_variables.find(attr_identifier)->second;

			std::string line = "@STORE_ATTR " + std::to_string(id)
				+ " (" + attr_identifier + ") " + std::to_string(ast->line_number);

			append_data(result, line, indentation);
		}
	}
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

		create_identifier_ast(array_refer_ast, array_refer_ast->identifier, result, array_refer_ast->line_number, indentation);

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
	case object_ast:
	{
		ClassAST* class_ast = ((ClassAST*)ast);
		scopes backup_scope = current_scope;
		current_scope = scope_class;
		current_class = class_ast->name;

		std::unordered_map<std::string, unsigned int> temp1 = {};
		std::unordered_map<std::string, std::string> temp2 = {};

		class_member_functions->insert(std::make_pair(current_class, temp1));
		class_member_functions_access_modifier->insert(std::make_pair(current_class, temp2));
		class_member_variables->insert(std::make_pair(current_class, temp1));
		class_member_variables_access_modifier->insert(std::make_pair(current_class, temp2));
		class_member_variables_type->insert(std::make_pair(current_class, temp2));

		unsigned int id = class_symbol.size();
		class_symbol.insert(std::make_pair(class_ast->name, id));

		if (ast->type == object_ast) {
			std::string name = class_ast->name;
			std::unordered_map<std::string, unsigned int>* member_variables = &class_member_variables->find(name)->second;
			std::unordered_map<std::string, unsigned int>* member_functions = &class_member_functions->find(name)->second;

			std::unordered_map<std::string, std::string>* member_variables_access_modifier
				= &class_member_variables_access_modifier->find(name)->second;
			std::unordered_map<std::string, std::string>* member_variables_type
				= &class_member_variables_type->find(name)->second;

			std::unordered_map<std::string, std::string>* member_functions_access_modifier
				= &class_member_functions_access_modifier->find(name)->second;

			member_variables->insert(std::make_pair("position", 0));
			member_variables_access_modifier->insert(std::make_pair("position", "public"));
			member_variables_type->insert(std::make_pair("position", "vector"));

			member_variables->insert(std::make_pair("width", 1));
			member_variables_access_modifier->insert(std::make_pair("width", "public"));
			member_variables_type->insert(std::make_pair("width", "number"));

			member_variables->insert(std::make_pair("height", 2));
			member_variables_access_modifier->insert(std::make_pair("height", "public"));
			member_variables_type->insert(std::make_pair("height", "number"));

			member_variables->insert(std::make_pair("rotation", 3));
			member_variables_access_modifier->insert(std::make_pair("rotation", "public"));
			member_variables_type->insert(std::make_pair("rotation", "number"));

			member_variables->insert(std::make_pair("sprite", 4));
			member_variables_access_modifier->insert(std::make_pair("sprite", "public"));
			member_variables_type->insert(std::make_pair("sprite", "string"));

			member_functions->insert(std::make_pair("render", 0));
			member_functions_access_modifier->insert(std::make_pair("render", "public"));
		}

		std::string parent_type;
		if (class_ast->parent_type == "")
			parent_type = "-1";
		else {
			parent_type = std::to_string(class_symbol[class_ast->parent_type]);
		}

		if (parent_type == "-1")
			parent_symbol.insert(std::make_pair(class_ast->name, "-1"));
		else
			parent_symbol.insert(std::make_pair(class_ast->name, class_ast->parent_type));

		current_class = class_ast->name;
		current_class_id = id;

		std::string object_type;

		if (ast->type == ast_type::class_ast)
			object_type = "CLASS";
		else if (ast->type == ast_type::scene_ast)
			object_type = "SCENE";
		else if (ast->type == ast_type::object_ast)
			object_type = "OBJECT";

		create_scope();
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
			std::unordered_map<std::string, unsigned int>* _class = &class_member_functions->find(current_class)->second;
			std::unordered_map<std::string, std::string>* _class_access_modifier = &class_member_functions_access_modifier->find(current_class)->second;

			std::string searcher = current_class;

			while (parent_symbol[searcher] != "-1") {
				searcher = parent_symbol[searcher];

				std::vector<BaseAST*> super_member_functions = parsed_class_data[searcher]->functions;

				for (int i = 0; i < super_member_functions.size(); i++) {
					FunctionDeclarationAST* _func = ((FunctionDeclarationAST*)super_member_functions[i]);

					if (_class->find(_func->function_name) == _class->end()) {
						_class->insert(std::make_pair(_func->function_name, i + (unsigned int)_class->size()));
						_class_access_modifier->insert(std::make_pair(_func->function_name, _func->access_modifier));
					}
				}

			}

			// check if there's function with the same name.
			id += (unsigned int)_class->size();
			_class->insert(std::make_pair(function_name, id));
			_class_access_modifier->insert(std::make_pair(function_name, function_declaration_ast->access_modifier));

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
		std::string identifier = _identifier_ast->identifier;

		Data* data = create_identifier_ast(_identifier_ast, identifier, result, _identifier_ast->line_number, indentation);
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
				id = get_local_variable_id(local_variable_symbol);

				local_variable_symbol->at(local_variable_symbol->size() - 1).insert(std::make_pair(variable_declaration_ast->names[i],
					Data{ id, variable_declaration_ast->var_types[i] }));

				break;
			}
			case scope_class:
				store_type = "@STORE_CLASS";

				std::unordered_map<std::string, unsigned int>* _class = &class_member_variables->find(current_class)->second;
				std::unordered_map<std::string, std::string>* _class_access_modifier = &class_member_variables_access_modifier->find(current_class)->second;
				std::unordered_map<std::string, std::string>* _class_type = &class_member_variables_type->find(current_class)->second;

				std::string searcher = current_class;

				while (parent_symbol[searcher] != "-1") {
					searcher = parent_symbol[searcher];

					std::vector<BaseAST*> super_member_variables = parsed_class_data[searcher]->variables;

					for (int i = 0; i < super_member_variables.size(); i++) {
						VariableDeclarationAST* _var = ((VariableDeclarationAST*)super_member_variables[i]);

						if (_class->find(_var->names[0]) != _class->end()) {
							_class->find(_var->names[0])->second = i + (unsigned int)_class->size();
							_class_access_modifier->find(_var->names[0])->second = _var->access_modifier;
							_class_type->find(_var->names[0])->second = _var->var_types[0];
						}
						else {
							_class->insert(std::make_pair(_var->names[0], i + (unsigned int)_class->size()));
							_class_access_modifier->insert(std::make_pair(_var->names[0], _var->access_modifier));
							_class_type->insert(std::make_pair(_var->names[0], _var->var_types[0]));
						}
					}

				}

				id = (unsigned int)_class->size();

				_class->insert(std::make_pair(variable_declaration_ast->names[i], id));
				_class_access_modifier->insert(std::make_pair(variable_declaration_ast->names[i], variable_declaration_ast->access_modifier));
				_class_type->insert(std::make_pair(variable_declaration_ast->names[i], variable_declaration_ast->var_types[i]));

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