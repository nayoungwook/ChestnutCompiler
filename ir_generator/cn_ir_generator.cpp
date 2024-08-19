#include "cn_ir_generator.h"

std::unordered_map<std::string, ClassAST*> parsed_class_data;
std::unordered_map<std::string, FunctionDeclarationAST*> parsed_function_data;
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
	builtin_function_symbol.insert(std::make_pair("background", 4));
}

void declare_builtin_variables() {
	Data data = { 0, "shader", false };
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

scopes get_scope_of_identifier(std::string const& identifier, BaseAST* identifier_ast) {
	std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();
	int id = get_local_variable_id(local_variable_symbol, identifier);

	if (identifier == "this" || identifier == "null") {
		return scope_local;
	}
	else {
		if (id == -1) {

			if (member_variable_data.find(current_class) != member_variable_data.end()
				&& member_variable_data[current_class].find(identifier) != member_variable_data[current_class].end()) {
				return scope_class;
			}
			else {
				std::string searcher = current_class;

				if (parsed_class_data.find(searcher) != parsed_class_data.end()) {
					while (parsed_class_data[searcher]->parent_type != "") {
						searcher = parsed_class_data[searcher]->parent_type;

						if (member_variable_data[searcher].find(identifier) != member_variable_data[searcher].end()) {
							return scope_class;
						}
					}
				}
			}

			if (global_variable_symbol.find(identifier) != global_variable_symbol.end()) {
				return scope_global;
			}
		}
		else {
			return scope_local;
		}
	}

	std::wstring w_identifier;
	w_identifier.assign(identifier.begin(), identifier.end());
	CHESTNUT_THROW_ERROR(L"Failed to find identifier : " + w_identifier, "FAILED_TO_FIND_IDENTIFIER", "004", identifier_ast->line_number);
}

scopes get_scope_of_function(std::string const& identifier, int line_number) {
	std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

	if (exist_in_symbol_table(global_function_symbol, identifier)) {
		return scope_global;
	}
	else if (exist_in_symbol_table(builtin_function_symbol, identifier)) {
		return scope_local;
	}

	if (member_function_data[current_class].find(identifier) != member_function_data[current_class].end()) {
		return scope_class;
	}
	else {
		std::string searcher = current_class;
		while (parsed_class_data[searcher]->parent_type != "") {
			searcher = parsed_class_data[searcher]->parent_type;

			if (member_function_data[searcher].find(identifier) != member_function_data[searcher].end()) {
				return scope_class;
			}
		}
	}

	std::wstring w_function_name;
	w_function_name.assign(identifier.begin(), identifier.end());
	CHESTNUT_THROW_ERROR(L"Failed to find function " + w_function_name, "FAILED_TO_FIND_FUNCTION_NAME", "008", line_number);
}

std::string create_identifier_ir(IdentifierAST* identifier_ast) {
	std::string result = "";

	if (identifier_ast->identifier == "this") {
		result = "@PUSH_THIS " + std::to_string(identifier_ast->line_number) + "\n";
	}
	else if (identifier_ast->identifier == "null") {
		result = "@PUSH_NULL " + std::to_string(identifier_ast->line_number) + "\n";
	}
	else {
		scopes scope = get_scope_of_identifier(identifier_ast->identifier, identifier_ast);

		if (scope == scope_local) {
			result = "@LOAD_LOCAL " + std::to_string(get_local_variable_id(local_variable_symbols.top(), identifier_ast->identifier))
				+ " (" + identifier_ast->identifier + ") " + std::to_string(identifier_ast->line_number) + "\n";

		}
		else if (scope == scope_class) { // find variable in current class declaration.
			std::string searcher = current_class;
			bool found_in_parent = false;
			while (parsed_class_data[searcher]->parent_type != "") {
				if (member_variable_data[searcher].find(identifier_ast->identifier) != member_variable_data[searcher].end()) { // member variable found.
					break;
				}

				searcher = parsed_class_data[searcher]->parent_type; // search for parent.
				found_in_parent = true;
			}

			// but, it was found in parnet and access modifier was private.
			if (member_variable_data[searcher][identifier_ast->identifier].access_modifier == "private" && found_in_parent) {
				std::wstring w_name;
				w_name.assign(identifier_ast->identifier.begin(), identifier_ast->identifier.end());

				CHESTNUT_THROW_ERROR(L"Variable you attempt to access \'" + w_name + L"\' is not public. create getter or setter "
					"\n\t\t\t(or just set it public.)"
					, "ACCESS_MODIFIER_IS_NOT_PUBLIC", "011", identifier_ast->line_number);
			}

			result = "@LOAD_CLASS " + std::to_string(get_parent_member_variable_size(searcher) + member_variable_data[searcher][identifier_ast->identifier].id)
				+ " (" + identifier_ast->identifier + ") " + std::to_string(identifier_ast->line_number);
		}
		else if (scope == scope_global) {
			result = "@LOAD_GLOBAL " + std::to_string(global_variable_symbol[identifier_ast->identifier].id)
				+ " (" + identifier_ast->identifier + ") " + std::to_string(identifier_ast->line_number);
		}

		result += "\n";

		if (identifier_ast->type == array_refer_ast) {
			for (int i = 0; i < ((ArrayReferAST*)identifier_ast)->indexes.size(); i++) {
				append_data(result, create_ir(((ArrayReferAST*)identifier_ast)->indexes[i], 0), 0);
				append_data(result, "@ARRAY_GET " + std::to_string(identifier_ast->line_number), 0);
			}
		}
	}

	result += "\n";

	result += create_attr_ir(identifier_ast, "lhs");

	return result;
}

Data get_data_of_variable(std::string const& identifier, BaseAST* data_ast) {
	scopes scope = get_scope_of_identifier(identifier, data_ast);

	if (scope == scope_local) {
		std::vector<std::unordered_map<std::string, Data>>* area = local_variable_symbols.top();
		for (int i = 0; i < area->size(); i++) {
			if (area->at(i).find(identifier) != area->at(i).end()) {
				std::unordered_map<std::string, Data>::iterator area_iterator;
				std::unordered_map<std::string, Data> found_area = area->at(i);

				for (area_iterator = found_area.begin(); area_iterator != found_area.end(); area_iterator++) {
					if (area_iterator->first == identifier)
						return area_iterator->second;
				}
			}
		}
	}
	else if (scope == scope_class) {
		Data result;
		result.id = member_variable_data[current_class][identifier].id;
		result.type = member_variable_data[current_class][identifier].type;
		result.is_array = member_variable_data[current_class][identifier].is_array;

		return result;
	}
	else if (scope == scope_global) {
		return global_variable_symbol[identifier];
	}
}

MemberVariableData get_member_variable_data(IdentifierAST* searcher, std::string const& type, bool is_array) {
	MemberVariableData member_variable;
	member_variable.id = -1;

	if (parsed_class_data[type] == nullptr && type != "vector") {
		std::cout << "We don\'t have \"" << type << "\" as a class. for read member var";
		exit(EXIT_FAILURE);
	}

	bool found_in_parent_memory = false;

	if (type == "vector") {
		if (searcher->identifier == "x")
			member_variable.id = 0;
		else if (searcher->identifier == "y")
			member_variable.id = 1;
		else if (searcher->identifier == "z")
			member_variable.id = 2;

		member_variable.name = searcher->identifier;
	}
	else {
		ClassAST* class_ast_searcher = parsed_class_data[type];

		while (true) {
			if (member_variable_data[class_ast_searcher->name].find(searcher->identifier) != member_variable_data[class_ast_searcher->name].end()) {
				member_variable.id
					= member_variable_data[class_ast_searcher->name][searcher->identifier].id + get_parent_member_variable_size(class_ast_searcher->name);

				member_variable.name = searcher->identifier;
				member_variable.access_modifier = member_variable_data[class_ast_searcher->name][searcher->identifier].access_modifier;
			}

			if (class_ast_searcher->parent_type == "") break;
			class_ast_searcher = parsed_class_data[class_ast_searcher->parent_type];
			found_in_parent_memory = true;
		}
	}

	if (member_variable.id == -1) {
		std::wstring w_type;
		w_type.assign(type.begin(), type.end());

		std::wstring w_member_function;
		w_member_function.assign(searcher->identifier.begin(), searcher->identifier.end());

		CHESTNUT_THROW_ERROR(L"Failed to find \'" + w_member_function + L"\' in a class \'" + w_type + L"\'", "FAILED_TO_LOAD_VARIABLE_FROM_CLASS", "007", searcher->line_number);
	}
	else if (type != "vector") {
		bool able_to_access = true;

		if (type == current_class && found_in_parent_memory) { // tried to access parent class.
			if (member_variable.access_modifier == "private") able_to_access = false;
		}
		else if (type != current_class) { // tried to access from outside of class.
			if (member_variable.access_modifier != "public") able_to_access = false;
		}

		if (!able_to_access) {
			std::wstring w_name;
			w_name.assign(searcher->identifier.begin(), searcher->identifier.end());

			CHESTNUT_THROW_ERROR(L"Variable you attempt to access \'" + w_name + L"\' is private. create getter or setter (or just set it public, protected.)"
				, "ACCESS_MODIFIER_IS_NOT_PUBLIC", "010", searcher->line_number);
		}
	}

	return member_variable;
}

MemberFunctionData get_member_function_of_array(FunctionCallAST* searcher) {
	MemberFunctionData member_function;

	member_function.id = -1;

	if (searcher->function_name == "push") {
		member_function.id = 0;
		member_function.name = "push";
	}
	else if (searcher->function_name == "size") {
		member_function.id = 1;
		member_function.name = "size";
	}
	else if (searcher->function_name == "remove") {
		member_function.id = 2;
		member_function.name = "remove";
	}

	return member_function;
}

MemberFunctionData get_member_function_data(FunctionCallAST* searcher, std::string const& type, bool is_array) {
	MemberFunctionData member_function;

	member_function.id = -1;

	bool found_in_parent_memory = false;


	if (is_array) {
		member_function = get_member_function_of_array(searcher);
	}
	else {
		if (parsed_class_data[type] == nullptr) {
			std::wstring w_type;
			w_type.assign(type.begin(), type.end());

			CHESTNUT_THROW_ERROR(L"There\'s no " + w_type + L" as a class", "FAIELD_TO_LOAD_VARIBLE_AS_CLASS", "006", searcher->line_number);
		}

		ClassAST* class_ast_searcher = parsed_class_data[type];

		while (true) {
			if (member_function_data[class_ast_searcher->name].find(searcher->function_name) != member_function_data[class_ast_searcher->name].end()) {
				member_function.id
					= member_function_data[class_ast_searcher->name][searcher->function_name].id + get_parent_member_variable_size(class_ast_searcher->name);
				member_function.name = searcher->function_name;
				member_function.access_modifier = member_function_data[class_ast_searcher->name][searcher->function_name].access_modifier;
			}

			if (class_ast_searcher->parent_type == "") break;
			class_ast_searcher = parsed_class_data[class_ast_searcher->parent_type];
			found_in_parent_memory = true;
		}
	}

	if (member_function.id == -1) {
		std::wstring w_type;
		w_type.assign(type.begin(), type.end());

		std::wstring w_member_function;
		w_member_function.assign(searcher->function_name.begin(), searcher->function_name.end());

		if (is_array)
			CHESTNUT_THROW_ERROR(L"Failed to find \'" + w_member_function + L"\' in a array", "FAILED_TO_LOAD_FUNCTION_FROM_CLASS", "007", searcher->line_number);
		else
			CHESTNUT_THROW_ERROR(L"Failed to find \'" + w_member_function + L"\' in a class \'" + w_type + L"\'", "FAILED_TO_LOAD_FUNCTION_FROM_CLASS", "007", searcher->line_number);
	}
	else if (!is_array) {
		bool able_to_access = true;

		if (type == current_class && found_in_parent_memory) { // tried to access parent class.
			if (member_function.access_modifier == "private") able_to_access = false;
		}
		else if (type != current_class) { // tried to access from outside of class.
			if (member_function.access_modifier != "public") able_to_access = false;
		}

		if (!able_to_access) {
			std::wstring w_name;
			w_name.assign(searcher->function_name.begin(), searcher->function_name.end());

			CHESTNUT_THROW_ERROR(L"Function you attempt to access \'" + w_name + L"\' is unaccessible. create getter or setter (or just set it public, protected.)"
				, "ACCESS_MODIFIER_IS_NOT_PUBLIC", "010", searcher->line_number);
		}
	}

	return member_function;
}

std::string get_type_of_attr_target(BaseAST* attr_target) {
	std::string type = "";

	// search for target.
	if (attr_target->type == ast_type::identifier_ast) {
		if (((IdentifierAST*)attr_target)->identifier == "this")
			type = current_class;
		else
			type = get_data_of_variable(((IdentifierAST*)attr_target)->identifier, attr_target).type;
	}
	else if (attr_target->type == ast_type::function_call_ast) {
		type = parsed_function_data[((FunctionCallAST*)attr_target)->function_name]->return_type;
	}
	else if (attr_target->type == ast_type::array_refer_ast) {
		type = get_data_of_variable(((ArrayReferAST*)attr_target)->identifier, attr_target).type;
	}

	return type;
}

bool is_attr_target_array(BaseAST* attr_target) {
	bool is_array;

	// search for target.
	if (attr_target->type == ast_type::identifier_ast) {
		if (((IdentifierAST*)attr_target)->identifier == "this")
			is_array = false;
		else
			is_array = get_data_of_variable(((IdentifierAST*)attr_target)->identifier, attr_target).is_array;
	}
	else if (attr_target->type == ast_type::function_call_ast) {
		std::string function_name = ((FunctionCallAST*)attr_target)->function_name;

		if (parsed_function_data.find(function_name) == parsed_function_data.end()) {
			std::wstring w_function_name;
			w_function_name.assign(function_name.begin(), function_name.end());

			CHESTNUT_THROW_ERROR(L"Failed to find function " + w_function_name, "FAILED_TO_FIND_FUNCTION_NAME", "008", attr_target->line_number);
		}

		is_array = parsed_function_data[function_name]->array_return_type != "";
	}
	else if (attr_target->type == ast_type::array_refer_ast) {
		is_array = get_data_of_variable(((ArrayReferAST*)attr_target)->identifier, attr_target).is_array;
	}

	return is_array && attr_target->type != array_refer_ast;
}

std::string create_attr_ir(IdentifierAST* identifier_ast, std::string const& lhs_rhs) {
	BaseAST* searcher = identifier_ast;
	BaseAST* attr_target = identifier_ast;
	std::string result;

	while (true) {
		if (searcher->attr == nullptr) break;

		if (searcher->attr->type == bin_expr_ast) {
			if (lhs_rhs == "lhs")
				searcher = ((BinExprAST*)searcher->attr)->lhs;
			else
				searcher = ((BinExprAST*)searcher->attr)->rhs;
		}
		else {
			searcher = searcher->attr;
		}

		std::string type = get_type_of_attr_target(attr_target);
		bool is_array = is_attr_target_array(attr_target);

		//append attribute data.
		if (searcher->type == ast_type::identifier_ast) {
			MemberVariableData member_variable = get_member_variable_data((IdentifierAST*)searcher, type, is_array);

			append_data(result, "@LOAD_ATTR " + std::to_string(member_variable.id)
				+ " (" + member_variable.name + ") " + std::to_string(searcher->line_number) + "\n", 0);
		}
		else if (searcher->type == ast_type::function_call_ast) {
			MemberFunctionData member_function = get_member_function_data((FunctionCallAST*)searcher, type, is_array);

			for (int i = ((FunctionCallAST*)searcher)->parameters.size() - 1; i >= 0; i--) {
				std::string param = create_ir(((FunctionCallAST*)searcher)->parameters[i], 0);
				append_data(result, param, 0);
			}

			append_data(result, "@CALL_ATTR " + std::to_string(member_function.id)
				+ " (" + member_function.name + ") "
				+ std::to_string(((FunctionCallAST*)searcher)->parameters.size()) + " " + std::to_string(searcher->line_number) + "\n", 0);
		}
		else if (searcher->type == ast_type::array_refer_ast) {
			MemberVariableData member_variable = get_member_variable_data((IdentifierAST*)searcher, type, is_array);

			append_data(result, "@LOAD_ATTR " + std::to_string(member_variable.id)
				+ " (" + member_variable.name + ") " + std::to_string(searcher->line_number) + "\n", 0);

			for (int i = 0; i < ((ArrayReferAST*)searcher)->indexes.size(); i++) {
				append_data(result, create_ir(((ArrayReferAST*)searcher)->indexes[i], 0), 0);
				append_data(result, "@ARRAY_GET " + std::to_string(searcher->line_number), 0);
			}
		}

		attr_target = searcher;
	}

	return result;
}

BaseAST* extract_last_ast(BaseAST* ast, std::string const& lhs_rhs) { // set nullptr to last ast of identifier.
	BaseAST* searcher = ast;
	BaseAST* attr_target = ast;

	while (true) {
		if (ast->attr == nullptr) return nullptr;
		if (ast->attr->type == bin_expr_ast) {
			if (lhs_rhs == "lhs")
				searcher = ((BinExprAST*)searcher->attr)->lhs;
			else
				searcher = ((BinExprAST*)searcher->attr)->rhs;
		}
		else {
			searcher = searcher->attr;
		}

		if (searcher->attr == nullptr) {
			attr_target->attr = nullptr;
			return searcher;
		}

		attr_target = searcher;
	}
	return nullptr;
}

BaseAST* get_last_ast(BaseAST* ast, std::string const& lhs_rhs) { // just 'get' the last ast of identifier.
	BaseAST* searcher = ast;
	BaseAST* attr_target = ast;

	if (ast->attr == nullptr) return ast;

	while (true) {
		if (ast->attr->type == bin_expr_ast) {
			if (lhs_rhs == "lhs")
				searcher = ((BinExprAST*)searcher->attr)->lhs;
			else
				searcher = ((BinExprAST*)searcher->attr)->rhs;
		}
		else {
			searcher = searcher->attr;
		}

		if (searcher->attr == nullptr) {
			return searcher;
		}

		attr_target = searcher;
	}
	return nullptr;
}

std::string create_assign_ir(BaseAST* ast, int indentation) {
	BinExprAST* bin_ast = ((BinExprAST*)ast);

	BaseAST* last_ast = extract_last_ast(bin_ast->lhs, "lhs"); // extract the last ast for make the ir of 'store'

	std::string result = "";

	if (bin_ast->rhs != nullptr) // if rhs is not already declared.
		result += create_ir(bin_ast->rhs, indentation);

	bool is_single_node = last_ast == nullptr;

	if (is_single_node) { // store ir for the single node.
		IdentifierAST* identifier_ast = (IdentifierAST*)bin_ast->lhs;

		scopes scope = get_scope_of_identifier(identifier_ast->identifier, identifier_ast);

		if (scope == scope_local) {
			result += "@STORE_LOCAL " + std::to_string(get_local_variable_id(local_variable_symbols.top(), identifier_ast->identifier))
				+ " (" + identifier_ast->identifier + ") " + std::to_string(identifier_ast->line_number) + "\n";
		}
		else if (scope == scope_class) {
			std::string searcher = current_class;
			while (parsed_class_data[searcher]->parent_type != "") {
				if (member_variable_data[searcher].find(identifier_ast->identifier) != member_variable_data[searcher].end()) {
					break;
				}

				searcher = parsed_class_data[searcher]->parent_type;
			}

			result += "@STORE_CLASS " + std::to_string(get_parent_member_variable_size(searcher) + member_variable_data[searcher][identifier_ast->identifier].id)
				+ " (" + identifier_ast->identifier + ") " + std::to_string(identifier_ast->line_number);
		}
		else if (scope == scope_global) {
			result += "@STORE_GLOBAL " + std::to_string(global_variable_symbol[identifier_ast->identifier].id)
				+ " (" + identifier_ast->identifier + ") " + std::to_string(identifier_ast->line_number);
		}
	}
	else { // store ir for attr.
		IdentifierAST* identifier_ast = (IdentifierAST*)bin_ast->lhs;
		result += create_identifier_ir(identifier_ast);

		BaseAST* attr_target = get_last_ast(identifier_ast, "lhs");

		std::string type = get_type_of_attr_target(attr_target);
		bool is_array = is_attr_target_array(attr_target);

		MemberVariableData member_variable = get_member_variable_data((IdentifierAST*)last_ast, type, is_array);

		result += "@STORE_ATTR " + std::to_string(member_variable.id)
			+ " (" + member_variable.name + ") " + std::to_string(attr_target->line_number) + "\n";
	}

	return result;
}

int get_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area, std::string const& obj_identifier) {
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
	return -1;
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

void create_super_call(BaseAST* ast, std::string& result, int indentation) {
	FunctionCallAST* function_call_ast = (FunctionCallAST*)ast;
	std::string line = "@SUPER_CALL " + std::to_string(function_call_ast->parameters.size())
		+ " " + std::to_string(ast->line_number);

	for (int i = function_call_ast->parameters.size() - 1; i >= 0; i--) {
		std::string param = create_ir(function_call_ast->parameters[i], 0);
		append_data(result, param, indentation);
	}

	append_data(result, line, 1);
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

		break;
	}

	case new_ast: {
		NewAST* new_ast = ((NewAST*)ast);

		for (int i = new_ast->parameters.size() - 1; i >= 0; i--) {
			std::string param = create_ir(new_ast->parameters[i], 0);
			append_data(result, param, indentation);
		}

		if (class_id.find(new_ast->class_name) == class_id.end()) {
			std::wstring class_name;
			class_name.assign(new_ast->class_name.begin(), new_ast->class_name.end());
			CHESTNUT_THROW_ERROR(L"Failed to find class name : " + class_name, "FAILED_TO_FIND_CLASS", "003", ast->line_number);
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
		current_class = "";

		break;
	}

	case constructor_declaration_ast: {
		ConstructorDeclarationAST* constructor_declaration_ast = (ConstructorDeclarationAST*)ast;

		std::string line = "$CONSTRUCTOR 0 (constructor) " + constructor_declaration_ast->access_modifier + " " + constructor_declaration_ast->return_type + " ";

		current_scope = scope_local;

		create_scope();

		std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

		for (int i = 0; i < constructor_declaration_ast->parameters.size(); i++) {
			VariableDeclarationAST* param = constructor_declaration_ast->parameters[i];

			line += param->names[0] + " " + param->var_types[0] + " ";

			local_variable_symbol->at(local_variable_symbol->size() - 1).insert(
				std::make_pair(constructor_declaration_ast->parameters[i]->names[i],
					Data{ (unsigned int)local_variable_symbol->size(), constructor_declaration_ast->parameters[i]->var_types[i] }
			));
		}

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

			id = get_parent_member_function_size(current_class) + member_function_data[current_class][function_name].id;

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

		line += function_declaration_ast->access_modifier + " " + function_declaration_ast->return_type + " ";

		for (int i = 0; i < function_declaration_ast->parameters.size(); i++) {
			line += function_declaration_ast->parameters[i]->names[0] + " ";
			line += function_declaration_ast->parameters[i]->var_types[0] + " ";

			local_variable_symbol->at(local_variable_symbol->size() - 1).insert(
				std::make_pair(function_declaration_ast->parameters[i]->names[0],
					Data{ (unsigned int)local_variable_symbol->size(), function_declaration_ast->parameters[i]->var_types[0] }
			));
		}

		line += " {\n";

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

			std::string type = variable_declaration_ast->var_types[i];

			if (variable_declaration_ast->array_var_types[i] != "")
				type = variable_declaration_ast->array_var_types[i];

			switch (current_scope) {
			case scope_global:
				store_type = "@STORE_GLOBAL";
				id = (unsigned int)global_variable_symbol.size();
				global_variable_symbol.insert(std::make_pair(variable_declaration_ast->names[i],
					Data{ id, type, variable_declaration_ast->array_var_types[i] != "" }
				));
				break;
			case scope_local: {
				std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol = local_variable_symbols.top();

				store_type = "@STORE_LOCAL";
				id = generate_local_variable_id(local_variable_symbol);

				local_variable_symbol->at(local_variable_symbol->size() - 1).insert(std::make_pair(variable_declaration_ast->names[i],
					Data{ id,  type,
					variable_declaration_ast->array_var_types[i] != "" }));

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

	case keyboard_ast: {
		KeyboardAST* keyboard_ast = ((KeyboardAST*)ast);

		append_data(result, "@KEYBOARD " + keyboard_ast->pressed_key + " " + std::to_string(ast->line_number), indentation);

		break;
	}

	case bin_expr_ast: {
		BinExprAST* bin_expr_ast = ((BinExprAST*)ast);

		if (bin_expr_ast->oper == "=") {
			append_data(result, create_assign_ir(ast, indentation), 0);
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

			std::string rhs = create_ir(bin_expr_ast->rhs, indentation);
			append_data(result, rhs, 0);

			std::string lhs = create_ir(bin_expr_ast->lhs, indentation);
			append_data(result, lhs, 0);

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
			append_data(result, create_assign_ir(bin_expr_ast, indentation), indentation);
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
			create_super_call(ast, result, indentation);
			break;
		}

		unsigned int function_id = -1;
		std::string call_type = "";

		scopes scope = get_scope_of_function(function_name, function_call_ast->line_number);

		if (scope == scope_class) { // call function in current class.
			function_id = member_function_data[current_class][function_name].id;
			call_type = "@CALL_CLASS";

			bool able_to_access = member_function_data[current_class][function_name].access_modifier != "private";

			if (!able_to_access) {
				std::wstring w_name;
				w_name.assign(function_name.begin(), function_name.end());

				CHESTNUT_THROW_ERROR(L"Function you attempt to access \'" + w_name + L"\' is private. create getter or setter (or just set it public, protected.)"
					, "ACCESS_MODIFIER_IS_NOT_PUBLIC", "010", ast->line_number);
			}
		}
		else if (exist_in_symbol_table(global_function_symbol, function_name)) { // call function in global area.
			function_id = global_function_symbol[function_name];
			call_type = "@CALL_GLOBAL";
		}
		else if (exist_in_symbol_table(builtin_function_symbol, function_name)) { // call function in builtin area.
			function_id = builtin_function_symbol[function_name];
			call_type = "@CALL_BUILTIN";
		}
		else {
			std::wstring w_function_name;
			w_function_name.assign(function_name.begin(), function_name.end());
			CHESTNUT_THROW_ERROR(L"Failed to find Function : " + w_function_name, "FAILED_TO_FIND_FUNCTION", "005", ast->line_number);
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