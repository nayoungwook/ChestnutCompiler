#pragma once

#include <iostream>
#include <unordered_map>
#include <sstream>
#include "../parser/cn_ast.h"
#include "../parser/cn_parser.h"

// declared based on serach priority
static std::unordered_map<std::wstring, unsigned int> global_function_symbol; // functions that exist in global area.
static std::unordered_map<std::wstring, unsigned int> builtin_function_symbol; // functions that exist in builtin area.

struct MemberFunctionData;
struct MemberVariableData;

enum scopes {
	scope_global, scope_local, scope_class,
};

static scopes current_scope = scope_global;

struct _Data {
	unsigned int id;
	std::wstring type;
	bool is_array;
} typedef Data;
Data get_data_of_variable(std::wstring const& identifier, BaseAST* data_ast);

static std::unordered_map<std::wstring, Data> global_variable_symbol; // functions that exist in global area.
static std::stack<std::vector<std::unordered_map<std::wstring, Data>>*> local_variable_symbols;// functions that exist in local area.

unsigned int generate_local_variable_id(std::vector<std::unordered_map<std::wstring, Data>>* area);
int get_local_variable_id(std::vector<std::unordered_map<std::wstring, Data>>* area, std::wstring const& obj_identifier);

unsigned int get_parent_member_variable_size(std::wstring const& class_name);
unsigned int get_parent_member_function_size(std::wstring const& class_name);

std::wstring create_attr_ir(IdentifierAST* identifier_ast, std::wstring const& lhs_rhs);

static unsigned int label_id = 0;
static std::wstring current_class = L"";
static unsigned int current_class_id = 0;

inline bool exist_in_symbol_table(std::unordered_map<std::wstring, unsigned int> area, std::wstring const& name);

// declare builtin
void declare_builtin_functions();
void declare_builtin_variables();

void append_data(std::wstring& target, std::wstring content, int indentation);

const std::wstring create_ir(BaseAST* ast, int indentation);
void create_super_call(BaseAST* ast, std::wstring& result, int indentation);

scopes get_scope_of_function(std::wstring const& identifier, int line_number);
scopes get_scope_of_identifier(std::wstring const& identifier, BaseAST* identifier_ast);

std::wstring create_assign_ir(BaseAST* ast, int indentation);
std::wstring create_identifier_ir(IdentifierAST* identifier_ast);

MemberVariableData get_member_variable_data(IdentifierAST* searcher, std::wstring const& type, bool is_array);

MemberFunctionData get_member_function_of_array(FunctionCallAST* searcher);
MemberFunctionData get_member_function_data(FunctionCallAST* searcher, std::wstring const& type, bool is_array);

std::wstring get_type_of_attr_target(BaseAST* attr_target);
bool is_attr_target_array(BaseAST* attr_target);

BaseAST* extract_last_ast(BaseAST* ast, std::wstring const& lhs_rhs);
BaseAST* get_last_ast(BaseAST* ast, std::wstring const& lhs_rhs);

void create_scope();
void destroy_scope();

const std::wstring integer_to_hex(int i);