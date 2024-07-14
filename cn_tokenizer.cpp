#include "cn_parser.h"

char get_char(std::string& line, int& _i) {
	if (_i + 1 < line.length()) {
		next_char = line[_i + 1];
	}
	return cur_char = line[_i++];
}

std::string get_number_literal(std::string& identifier, std::string& line, int& i) {
	std::string number_string = "";

	while (isdigit(cur_char) || cur_char == '.') {
		number_string += cur_char;

		if (next_char == 'f') {
			number_string += 'f';
			i++;
			break;
		}

		if ((isdigit(next_char) || next_char == '.'))
			get_char(line, i);
		else
			break;
	}

	identifier = number_string;
	return number_string;
}

std::vector<Token*> tokenize(std::string line, int line_number) {
	std::vector<Token*> result_tokens;

	int type = 0;
	std::string identifier = "";
	double number = 0;

	int i = 0;

	while (i < line.length()) {
		identifier = "";

		get_char(line, i);

		if (isspace(cur_char)) continue;

		if (isalpha(cur_char) || cur_char == '_') {
			type = tok_identifier;
			while (isalnum(next_char) || next_char == '_') {
				if (i >= line.length()) break;
				identifier += cur_char;
				get_char(line, i);
			}
			identifier += cur_char;

			if (identifier == "func") type = tok_func;
			if (identifier == "if") type = tok_if;
			if (identifier == "else") type = tok_else;
			if (identifier == "for") type = tok_for;
			if (identifier == "while") type = tok_while;

			if (identifier == "var") type = tok_var;

			if (identifier == "number") type = tok_number;
			if (identifier == "int") type = tok_int;
			if (identifier == "float") type = tok_float;
			if (identifier == "bool") type = tok_bool;
			if (identifier == "char") type = tok_char;
			if (identifier == "string") type = tok_string;

			if (identifier == "cast") type = tok_cast;

			if (identifier == "public") type = tok_pubilc;
			if (identifier == "private") type = tok_private;
			if (identifier == "protected") type = tok_protected;
			if (identifier == "static") type = tok_static;
			if (identifier == "const") type = tok_const;

			if (identifier == "return") type = tok_return;
			if (identifier == "class") type = tok_class;

			if (identifier == "new") type = tok_new;

			if (identifier == "true") type = tok_true;
			if (identifier == "false") type = tok_false;
			if (identifier == "extends") type = tok_extends;
			if (identifier == "constructor") type = tok_constructor;
			if (identifier == "scene") type = tok_scene;
			if (identifier == "object") type = tok_object;
			if (identifier == "vector") type = tok_vector;
		}
		else if (isdigit(cur_char)) {
			type = tok_identifier_number;
			get_number_literal(identifier, line, i);
		}
		else if (cur_char == '\"') {
			type = tok_string_identifier;
			bool is_inner_quote = false;

			identifier = "";

			while (true) {
				identifier += cur_char;

				if (cur_char == '\\' && next_char == '\"') {
					is_inner_quote = true;
				}

				get_char(line, i);

				if (cur_char == '\"') {
					if (is_inner_quote) {
						is_inner_quote = false;
						continue;
					}
					else {
						identifier += cur_char;
						break;
					}
				}
			}
		}
		// TODO : remove this comment token it occurs error
		else if (cur_char == '/' && next_char == '/') {
			type = tok_comment;
			identifier = "(comment)";
			//result_tokens.push_back(new Token(type, identifier, number, line_number));
			return result_tokens;
		}
		else if (cur_char == '&') {
			if (next_char == '&') {
				type = tok_and;
				identifier = "&&";
				i++;
			}
		}
		else if (cur_char == '|') {
			if (next_char == '|') {
				type = tok_or;
				identifier = "||";
				i++;
			}
		}
		else if (cur_char == '=') {
			if (next_char == '=') {
				type = tok_equal;
				identifier = "==";
				i++;
			}
			else {
				type = tok_assign;
				identifier = "=";
			}
		}
		else if (cur_char == '.') {
			type = tok_dot;
			identifier = ".";
		}
		else if (cur_char == '<') {
			if (next_char == '=') {
				type = tok_eq_greater;
				identifier = "<=";
				i++;
			}
			else {
				type = tok_greater;
				identifier = "<";
			}
		}
		else if (cur_char == '>') {
			if (next_char == '=') {
				type = tok_eq_greater;
				identifier = ">=";
				i++;
			}
			else {
				type = tok_greater;
				identifier = ">";
			}
		}
		else if (cur_char == '!') {
			if (next_char == '=') {
				type = tok_not_equal;
				identifier = "!=";
				i++;
			}
			else {
				type = tok_not;
				identifier = "!";
			}
		}
		else if (cur_char == ')') {
			type = tok_r_paren;
			identifier = ")";
		}
		else if (cur_char == '(') {
			type = tok_l_paren;
			identifier = "(";
		}
		else if (cur_char == '}') {
			type = tok_r_bracket;
			identifier = "}";
		}
		else if (cur_char == '{') {
			type = tok_l_bracket;
			identifier = "{";
		}
		else if (cur_char == ']') {
			type = tok_r_sq_bracket;
			identifier = "]";
		}
		else if (cur_char == '[') {
			type = tok_l_sq_bracket;
			identifier = "[";
		}
		else if (cur_char == '^') {
			type = tok_pow;
			identifier = "^";
		}
		else if (cur_char == '+') {
			if (next_char == '+') {
				type = tok_incre;
				identifier = "++";
				i++;
			}
			else if (next_char == '=') {
				type = tok_bin_incre;
				identifier = "+=";
				i++;
			}
			else {
				type = tok_pls;
				identifier = "+";
			}
		}
		else if (cur_char == '-') {
			if (next_char == '-') {
				type = tok_decre;
				identifier = "--";
				i++;
			}
			else if (next_char == '=') {
				type = tok_bin_decre;
				identifier = "-=";
				i++;
			}
			else {
				type = tok_min;
				identifier = "-";
			}
		}
		else if (cur_char == '*') {
			if (next_char == '=') {
				type = tok_bin_mul;
				identifier = "*=";
				i++;
			}
			else {
				type = tok_mul;
				identifier = "*";
			}
		}
		else if (cur_char == '/') {
			if (next_char == '=') {
				type = tok_bin_div;
				identifier = "/=";
				i++;
			}
			else {
				type = tok_div;
				identifier = "/";
			}
		}
		else if (cur_char == '#') {

			if (next_char == ' ') {
				type = tok_sharp;
				identifier = "#";
			}
			else {
				while (isalnum(next_char) || next_char == '_') {
					if (i >= line.length()) break;
					identifier += cur_char;
					get_char(line, i);
				}
				identifier += cur_char;

				if (identifier == "#import")
					type = tok_import;
				else if (identifier == "#load")
					type = tok_load;
			}

		}
		else if (cur_char == ';') {
			type = tok_semi_colon;
			identifier = ";";
		}
		else if (cur_char == ':') {
			type = tok_colon;
			identifier = ":";
		}
		else if (cur_char == ',') {
			type = tok_comma;
			identifier = ",";
		}
		else if (cur_char == '%') {
			type = tok_mod;
			identifier = "%";
		}

		result_tokens.push_back(new Token(type, identifier, line_number));
	}

	return result_tokens;
}
