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

static std::unordered_map<std::string, unsigned int> class_symbol; // functions that exist in builtin area.
static std::unordered_map<std::string, std::string> parent_symbol; // functions that exist in builtin area.

static std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>>* class_member_variables
= new std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>>; // memeber variables.

static std::unordered_map<std::string, std::unordered_map<std::string, std::string>>* class_member_variables_type
= new std::unordered_map<std::string, std::unordered_map<std::string, std::string>>; // memeber variables.

static std::unordered_map<std::string, std::unordered_map<std::string, std::string>>* class_member_variables_access_modifier
= new std::unordered_map<std::string, std::unordered_map<std::string, std::string>>; // access modifier of memeber variables.

static std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>>* class_member_functions
= new std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>>; // memeber functions.

static std::unordered_map<std::string, std::unordered_map<std::string, std::string>>* class_member_functions_access_modifier
= new std::unordered_map<std::string, std::unordered_map<std::string, std::string>>; // access modifier of memeber functions.

enum scopes {
	scope_global, scope_local, scope_class,
};

static scopes current_scope = scope_global;

struct _Data {
	unsigned int id;
	std::string type;
} typedef Data;

static std::unordered_map<std::string, Data> global_variable_symbol; // functions that exist in global area.

static std::stack<
	std::vector<std::unordered_map<std::string, Data>>*> local_variable_symbols;// functions that exist in builtin area.
Data* get_local_variable(std::vector<std::unordered_map<std::string, Data>>* area, std::string const& obj_identifier);
Data* get_data(std::vector<std::unordered_map<std::string, Data>>* local_variable_symbol, std::string const& identifier);

unsigned int get_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area);
unsigned int get_local_variable_id(std::vector<std::unordered_map<std::string, Data>>* area, std::string const& obj_identifier);

static unsigned int label_id = 0;
static std::string current_class = "";
static unsigned int current_class_id = 0;

std::string get_store_type(unsigned int& id, BaseAST* last_ast);
inline bool exist_in_symbol_table(std::unordered_map<std::string, unsigned int> area, std::string const& name);

// declare builtin
void declare_builtin_functions();

std::pair<scopes, Data*> get_memory_scope(std::string const& identifier);

void append_data(std::string& target, std::string content, int indentation);

const std::string create_ir(BaseAST* ast, int indentation);

void create_assign_ir(BaseAST* ast, std::string& result, int indentation);
Data* create_identifier_ast(BaseAST* identifier_ast, std::string const& identifier, std::string& result, int line_number, int indentation);

void create_scope();
void destroy_scope();

const std::string integer_to_hex(int i);