#pragma once

#include <vector>
#include <iostream>
#include <map>
#include "../tokenizer/cn_token.h"
#include <stack>
#include <set>

enum ast_type {
	base_ast = 0,
	import_ast = 1,
	variable_declaration_ast = 2,
	function_declaration_ast = 3,
	function_call_ast = 4,
	return_ast = 5,
	if_statement_ast = 6,
	for_statement_ast = 7,
	bin_expr_ast = 8,
	number_ast = 9,
	identifier_ast = 10,
	string_literal_ast = 11,
	operator_ast = 12,
	paren_ast = 13,
	class_ast = 14,
	new_ast = 15,
	character_ast = 16,
	bool_ast = 17,
	while_statement_ast = 18,
	array_ast = 19,
	array_refer_ast = 20,
	array_declaration_ast = 21,
	constructor_declaration_ast = 22,
	not_ast = 23,
	scene_ast = 24,
	object_ast = 25,
	vector_declaration_ast = 26,
	cast_ast = 27,
	keyboard_ast = 28,
	load_ast = 29,
};

class BaseAST {
private:
public:
	virtual ~BaseAST() {};
	BaseAST() {};
	int type = base_ast;
	int line_number = 0;
	BaseAST* attr = nullptr;
};

class NumberAST : public BaseAST {
private:
public:
	std::string number_string = "0";

	NumberAST(const std::string& number) : number_string(number) {
		type = number_ast;
	}
};

class ArrayAST : public BaseAST {
private:
public:
	size_t size;
	std::vector<BaseAST*> content;

	ArrayAST(size_t size, std::vector<BaseAST*>& content) : size(size), content(content) {
		type = array_ast;
	}
};

class IdentifierAST : public BaseAST {
private:
public:
	std::string identifier; // TODO : maybe do some type check later.
	IdentifierAST(const std::string& identifier) : identifier(identifier) {
		type = identifier_ast;
	}
};

class StringLiteralAST : public BaseAST {
private:
public:
	std::string str_literal; // TODO : maybe do some type check later.
	StringLiteralAST(const std::string& str_literal) : str_literal(str_literal) {
		type = string_literal_ast;
	}
};

class CharacterAST : public BaseAST {
private:
public:
	char character; // TODO : maybe do some type check later.
	CharacterAST(char character) : character(character) {
		type = character_ast;
	}
};

class BoolAST : public BaseAST {
private:
public:
	bool bool_data; // TODO : maybe do some type check later.
	BoolAST(bool bool_data) : bool_data(bool_data) {
		type = bool_ast;
	}
};

class ImportAST : public BaseAST {
private:
public:
	std::string import_type;
	ImportAST(const std::string& import_type) : import_type(import_type) {
		type = import_ast;
	};
};

class LoadAST : public BaseAST {
private:
public:
	std::string path, name;
	LoadAST(std::string const& name, std::string const& path) : path(path), name(name) {
		type = load_ast;
	};
};

class VariableDeclarationAST : public BaseAST {
private:
public:
	bool is_static = false;
	std::string access_modifier = "default";
	std::vector<std::string> var_types, names, array_var_types;
	std::vector<BaseAST*> declarations;
	size_t var_count;

	VariableDeclarationAST(std::vector<std::string>& var_types, std::vector<std::string>& names,
		std::vector<BaseAST*>& declarations, std::vector<std::string>& array_var_types, size_t var_count) : var_types(var_types), names(names), declarations(declarations), var_count(var_count), array_var_types(array_var_types) {
		type = variable_declaration_ast;
	}
};

class FunctionDeclarationAST : public BaseAST {
private:
public:
	bool is_static = false;
	std::string return_type, function_name, access_modifier = "default";
	std::vector<VariableDeclarationAST*> parameter;
	std::vector<BaseAST*> body;

	FunctionDeclarationAST(const std::string& function_name, std::vector<VariableDeclarationAST*>& parameter, const std::string& return_type)
		: function_name(function_name), return_type(return_type) {
		this->parameter = parameter;

		type = function_declaration_ast;
	};
};

class ConstructorDeclarationAST : public FunctionDeclarationAST {
private:
public:
	ConstructorDeclarationAST(std::vector<VariableDeclarationAST*>& parameter)
		: FunctionDeclarationAST("constructor", parameter, "void") {
		this->parameter = parameter;

		type = constructor_declaration_ast;
	};
};

class FunctionCallAST : public BaseAST {
private:
public:
	std::string function_name;
	std::vector<BaseAST*> parameters;
	FunctionCallAST(const std::string& function_name, std::vector<BaseAST*>& parameters)
		: function_name(function_name) {
		this->parameters = parameters;

		type = function_call_ast;
	}
};

class ReturnAST : public BaseAST {
private:
public:
	BaseAST* expression;
	ReturnAST(BaseAST* expression) {
		this->expression = expression;

		type = return_ast;
	}
};

enum if_statement_type {
	statement_if = 0, statement_else = 1, statement_elif = 2,
};

class IfStatementAST : public BaseAST {
private:
public:
	BaseAST* condition;
	std::vector<BaseAST*> body;
	std::vector<IfStatementAST*> additional_statements;
	if_statement_type statement_type = statement_if;

	IfStatementAST(BaseAST* condition)
		: condition(condition) {
		type = if_statement_ast;
	}
};

class ForStatementAST : public BaseAST {
private:
public:
	BaseAST* init, * condition, * step;
	std::vector<BaseAST*> body;

	ForStatementAST(BaseAST* init, BaseAST* condition, BaseAST* step)
		: init(init), condition(condition), step(step) {
		type = for_statement_ast;
	}
};

class WhileStatementAST : public BaseAST {
private:
public:
	BaseAST* condition;
	std::vector<BaseAST*> body;

	WhileStatementAST(BaseAST* condition)
		: condition(condition) {
		type = while_statement_ast;
	}
};

class BinExprAST : public BaseAST {
private:
public:
	std::string oper;
	BaseAST* lhs = nullptr, * rhs = nullptr;

	BinExprAST(std::string const& oper, BaseAST* lhs, BaseAST* rhs)
		: oper(oper), lhs(lhs), rhs(rhs) {
		type = bin_expr_ast;
	}
};

class OperatorAST : public BaseAST {
private:
public:
	std::string oper;

	OperatorAST(std::string const& oper) : oper(oper) {
		type = operator_ast;
	}
};

class ParenAST : public BaseAST {
private:
public:
	std::string paren;

	ParenAST(std::string const& paren) : paren(paren) {
		type = paren_ast;
	}
};

class ClassAST : public BaseAST {
private:
public:
	std::string name;
	std::vector<BaseAST*> functions;
	std::vector<BaseAST*> variables;
	std::string parent_type = "";
	std::vector<ConstructorDeclarationAST*> constructor;

	ClassAST(std::string& name, std::string& parent_type) : name(name), parent_type(parent_type) {
		type = class_ast;
	}
};

class SceneAST : public ClassAST {
private:
public:
	SceneAST(std::string& name, std::string& parent_type) : ClassAST(name, parent_type) {
		type = scene_ast;
	}
};

class ObjectAST : public ClassAST {
private:
public:
	ObjectAST(std::string& name, std::string& parent_type) : ClassAST(name, parent_type) {
		type = object_ast;
	}
};

class NewAST : public BaseAST {
private:
public:
	std::string class_name;
	std::vector<BaseAST*> parameters;

	NewAST(std::string& class_name) : class_name(class_name) {
		type = new_ast;
	}
};

class ArrayReferAST : public IdentifierAST {
private:
public:
	std::vector<BaseAST*> indexes;

	ArrayReferAST(std::string& identifier, std::vector<BaseAST*>& indexes) : IdentifierAST(identifier), indexes(indexes) {
		type = array_refer_ast;
	}
};

class ArrayDeclarationAST : public BaseAST {
private:
public:
	std::vector<BaseAST*> elements;
	size_t size;

	ArrayDeclarationAST(std::vector<BaseAST*>& elements) : elements(elements), size(elements.size()) {
		type = array_declaration_ast;
	}
};

class NotAST : public BaseAST {
private:
public:
	BaseAST* expression;

	NotAST(BaseAST* expression) : expression(expression) {
		type = not_ast;
	}
};

class VectorDeclarationAST : public BaseAST {
private:
public:
	std::vector<BaseAST*> elements;

	VectorDeclarationAST(std::vector<BaseAST*>& elements) : elements(elements) {
		type = vector_declaration_ast;
	}
};

class CastAST : public BaseAST {
private:
public:
	BaseAST* target_ast;
	std::string cast_type = "";

	CastAST(std::string& cast_type, BaseAST* target_ast) : cast_type(cast_type), target_ast(target_ast) {
		type = cast_ast;
	}
};

class KeyboardAST : public BaseAST {
private:
public:
	std::string pressed_key = "";

	KeyboardAST(std::string& pressed_key) : pressed_key(pressed_key) {
		type = keyboard_ast;
	}
};

std::vector<Token*> get_parameter_tokens(std::vector<Token*>& tokens);
std::vector<std::vector<Token*>> get_function_parameter_tokens(std::vector<Token*>& tokens);
std::vector<Token*> get_block_tokens(std::vector<Token*>& tokens);

int get_nodes_priority(const BaseAST* node);
std::vector<BaseAST*> to_postfix(std::vector<BaseAST*>& nodes);
