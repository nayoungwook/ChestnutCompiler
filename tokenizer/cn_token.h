#pragma once

#include <map>
#include <iostream>

enum token_type {
	tok_null = -1,

	tok_identifier = 0, tok_identifier_number = 1,

	tok_equal = 2, tok_not_equal = 3, tok_greater = 4, tok_lesser = 5, tok_eq_greater = 6, tok_eq_lesser = 7,

	tok_pls = 8, tok_min = 9, tok_mul = 10, tok_div = 11,

	tok_l_paren = 12, tok_r_paren = 13,

	tok_l_bracket = 14, tok_r_bracket = 15,

	tok_l_sq_bracket = 16, tok_r_sq_bracket = 17,

	tok_assign = 18, tok_sharp = 19,

	tok_comment = 19,

	tok_semi_colon = 20, tok_colon = 21,

	tok_func = 22, tok_if = 23, tok_else = 24, tok_for = 25, tok_while = 26,

	tok_number = 28, tok_char = 29, tok_string = 30,

	tok_import = 31, tok_basic = 32,

	tok_string_identifier = 33,

	tok_pubilc = 34, tok_private = 35, tok_protected = 36,

	tok_static = 37, tok_const = 38,

	tok_comma = 39, tok_return = 40, tok_mod = 41, tok_not = 42,

	tok_and = 43, tok_or = 44,

	tok_incre = 45, tok_decre = 46,

	tok_bin_incre = 47, tok_bin_decre = 48, tok_bin_mul = 49, tok_bin_div = 50,

	tok_class = 51, tok_dot = 52,

	tok_new = 53,

	tok_true = 54, tok_false = 55,

	tok_pow = 56,

	tok_array = 57, tok_extends = 58, tok_constructor = 59,

	tok_scene = 60, tok_object = 61, tok_vector = 62,

	tok_cast = 63,

	tok_int = 64, tok_float = 65, tok_bool = 66, tok_load = 67,
};

static std::map<int, std::string> token_string = {
	{tok_null, "unexpected"},
	{tok_identifier, "identifier"},
	{tok_identifier_number, "number"},
	{tok_equal, "=="},
	{tok_not_equal, "!="},
	{tok_greater, ">"},
	{tok_lesser, "<"},
	{tok_eq_greater, ">="},
	{tok_eq_lesser, "<="},
	{tok_pls, "+"},
	{tok_min, "-"},
	{tok_div, "/"},
	{tok_mul, "*"},
	{tok_pow, "^"},
	{tok_mod, "%"},
	{tok_l_paren, "("},
	{tok_r_paren, ")"},
	{tok_l_bracket, "{"},
	{tok_r_bracket, "}"},
	{tok_l_sq_bracket, "["},
	{tok_r_sq_bracket, "]"},
	{tok_assign, "="},
	{tok_sharp, "#"},
	{tok_semi_colon, ";"},
	{tok_colon, ":"},
	{tok_func, "func"},
	{tok_if, "if"},
	{tok_else, "else"},
	{tok_for, "for"},
	{tok_while, "while"},
	{tok_number, "number"},
	{tok_char, "char"},
	{tok_string, "string"},
	{tok_import, "#import"},
	{tok_load, "#load"},
	{tok_basic, "basic"},
	{tok_string_identifier, "string identifier"},
	{tok_pubilc, "public"},
	{tok_private, "private"},
	{tok_protected, "protected"},
	{tok_static, "static"},
	{tok_const, "const"},
	{tok_comma, ","},
	{tok_return, "return"},
	{tok_or, "||"},
	{tok_incre, "++"},
	{tok_decre, "--"},
	{tok_bin_incre, "+="},
	{tok_bin_decre, "-="},
	{tok_bin_mul, "*="},
	{tok_bin_div, "/="},
	{tok_class, "class"},
	{tok_dot, "."},
	{tok_new, "new"},
	{tok_true, "true"},
	{tok_false, "false"},
	{tok_array, "array"},
	{tok_extends, "extends"},
	{tok_constructor, "constructor"},
	{tok_scene, "scene"},
	{tok_object, "object"},
	{tok_vector, "vector"},
	{tok_cast, "cast"},
	{tok_int, "int"},
	{tok_float, "float"},
	{tok_bool, "bool"},
};

class Token
{
public:
	int type = 0;
	std::string identifier = 0;
	int line = 0;

	~Token() = default;

	Token(int type, std::string identifier, int line_number)
		: type(type), identifier(identifier), line(line_number) {}
};