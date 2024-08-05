#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include "../parser/cn_ast.h"
#include "../parser/cn_parser.h"

// declared based on serach priority
static std::unordered_map<std::string, unsigned int> global_function_symbol; // functions that exist in global area.
static std::unordered_map<std::string, unsigned int> builtin_function_symbol; // functions that exist in builtin area.

struct MemberFunctionData;
struct MemberVariableData;

enum scopes {
	scope_global, scope_local, scope_class,
};

static scopes current_scope = scope_global;

struct _Data {
	unsigned int id;
	std::string type;
	bool is_array;
} typedef Data;
Data get_data_of_variable(std::string const& identifier, BaseAST* data_ast);

static std::unordered_map<std::string, Data> global_variable_symbol; // functions that exist in global area.
static std::stack<std::vector<std::unordered_map<std::string, Data>>*> local_variable_symbols;// functions that exist in local area.

unsigned int generate_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area);
int get_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area, std::string const& obj_identifier);

unsigned int get_parent_member_variable_size(std::string const& class_name);
unsigned int get_parent_member_function_size(std::string const& class_name);

std::string create_attr_ir(IdentifierAST* identifier_ast, std::string const& lhs_rhs);

static unsigned int label_id = 0;
static std::string current_class = "";
static unsigned int current_class_id = 0;

inline bool exist_in_symbol_table(std::unordered_map<std::string, unsigned int> area, std::string const& name);

// declare builtin
void declare_builtin_functions();

void append_data(std::string& target, std::string content, int indentation);

const std::string create_ir(BaseAST* ast, int indentation);

scopes get_scope_of_function(std::string const& identifier);
scopes get_scope_of_identifier(std::string const& identifier, BaseAST* identifier_ast);

std::string create_assign_ir(BaseAST* ast, int indentation);
std::string create_identifier_ir(IdentifierAST* identifier_ast);

MemberVariableData get_member_variable_data(IdentifierAST* searcher, std::string const& type, bool is_array);

MemberFunctionData get_array_member_function_data(FunctionCallAST* searcher);
MemberFunctionData get_member_function_data(FunctionCallAST* searcher, std::string const& type, bool is_array);

std::string get_type_of_attr_target(BaseAST* attr_target);
bool is_attr_target_array(BaseAST* attr_target);

BaseAST* extract_last_ast(BaseAST* ast, std::string const& lhs_rhs);
BaseAST* get_last_ast(BaseAST* ast, std::string const& lhs_rhs);

void create_scope();
void destroy_scope();

const std::string integer_to_hex(int i);